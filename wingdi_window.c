/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 2007-2011 Elizabeth M. Smith, Sara Golemon, Tom Rogers |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Elizabeth M. Smith <auroraeosrose@php.net>                   |
  |         Mark G. Skilbeck <markskilbeck@php.net>                      |
  +----------------------------------------------------------------------+
*/

#include "php_wingdi.h"
#include "zend_exceptions.h"

PHP_WINGDI_API zend_class_entry *ce_wingdi_window;
static zend_object_handlers     wingdi_window_object_handlers;
static zend_function            wingdi_window_constructor_wrapper;

/* ----------------------------------------------------------------
  Win\Gdi\Window Userland API                                   

  Window stuff that is GDI specific is implemented here, BeginPaint
  and EndPaint, for example. The WM_* GDI constants, however, are
  implemented by Win\Gui\Window.
------------------------------------------------------------------*/

/** {{{ proto Win\Gdi\DeviceContext Win\Gdi\Window::beginPaint(array & paint_data)
		Prepares the specified window for painting. The paint_data reference variable
		receives information about the painting.
*/
PHP_METHOD( WinGdiWindow, beginPaint )
{
	wingdi_devicecontext_object * dc_object,
								* dc_object_display;
	wingdi_window_object        * window_object = zend_object_store_get_object( getThis() TSRMLS_CC );
	zval                        * paint_data, 
	                            * paint_data_rect = NULL,
								* paint_data_hdc  = NULL;
	PAINTSTRUCT                   paint_struct;
	HDC                           hdc;

	WINGDI_ERROR_HANDLING();
	if ( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "z", &paint_data ) == FAILURE )
		return;
	WINGDI_RESTORE_ERRORS();

	hdc = BeginPaint( window_object->window_handle, &paint_struct );
	if ( !hdc )
	{
		wingdi_create_error( GetLastError(), ce_wingdi_exception TSRMLS_CC );
		return;
	}

	// Set up return value for a DC object
	object_init_ex( return_value, ce_wingdi_devicecontext );
	dc_object = ( wingdi_devicecontext_object * ) zend_objects_get_address( return_value TSRMLS_CC );
	dc_object->hdc = hdc;
	dc_object->constructed = 1;

	// Empty whatever the paint_data zval contains, and store in it the data from paint_struct
	zval_dtor( paint_data );
	array_init( paint_data );
	// Set up the hdc element object
	MAKE_STD_ZVAL( paint_data_hdc );
	object_init_ex( paint_data_hdc, ce_wingdi_devicecontext );
	dc_object_display = ( wingdi_devicecontext_object * ) zend_objects_get_address( paint_data_hdc TSRMLS_CC );
	dc_object_display->hdc = paint_struct.hdc;
	dc_object_display->constructed = 1;
	add_assoc_zval( paint_data, "hdc",   paint_data_hdc );
	add_assoc_bool( paint_data, "erase", paint_struct.fErase );
	// paint_struct.rcPaint is a RECT structure which maps to a PHP array
	MAKE_STD_ZVAL( paint_data_rect );
	array_init( paint_data_rect );
	add_next_index_long( paint_data_rect, paint_struct.rcPaint.left );
	add_next_index_long( paint_data_rect, paint_struct.rcPaint.top );
	add_next_index_long( paint_data_rect, paint_struct.rcPaint.right );
	add_next_index_long( paint_data_rect, paint_struct.rcPaint.bottom );
	add_assoc_zval( paint_data, "paint", paint_data_rect );
}
/** }}} */

/** {{{ proto bool Win\Gdi\Window::endPaint( paint_data );
		Marks the end of painting in the specified window.
*/
PHP_METHOD( WinGdiWindow, endPaint )
{
	wingdi_devicecontext_object * dc_object;
	wingdi_window_object        * window_object = zend_object_store_get_object( getThis() TSRMLS_CC );
	HashTable                   * paint_data;
	PAINTSTRUCT                   paint_struct;

	WINGDI_ERROR_HANDLING();
	if ( zend_parse_parameters( ZEND_NUM_ARGS() TSRMLS_CC, "h", &paint_data ) == FAILURE )
		return;
	WINGDI_RESTORE_ERRORS();

	// Error checking
	// Not sure about the error messages
	if ( ! zend_hash_exists( paint_data, "hdc", strlen( "hdc" ) + 1 ) )
	{
		php_error( E_ERROR, "no 'hdc' element found in array" );
		return;
	}
	else if ( ! zend_hash_exists( paint_data, "erase", strlen( "erase" ) + 1 ) )
	{
		php_error( E_ERROR, "no 'erase' element found in array" );
		return;
	}
	else if ( ! zend_hash_exists( paint_data, "paint", strlen( "paint" ) + 1 ) )
	{
		php_error( E_ERROR, "no 'paint' element found in array" );
		return;
	}
	else
	{
		zval ** hdc_element,
			 ** erase_element,
			 ** paint_element,
			 ** left = NULL, ** top = NULL, ** right = NULL, ** bottom = NULL;
		HashTable * paint_rect;

		zend_hash_find( paint_data, "hdc", strlen( "hdc" ) + 1, ( void ** ) &hdc_element );
		dc_object = ( wingdi_devicecontext_object * ) zend_objects_get_address( * hdc_element TSRMLS_CC );
		paint_struct.hdc = dc_object->hdc;

		zend_hash_find( paint_data, "erase", strlen( "erase" ) + 1, ( void ** ) &erase_element );
		paint_struct.fErase = Z_BVAL_PP( erase_element );

		zend_hash_find( paint_data, "paint", strlen( "paint" ) + 1, ( void ** ) &paint_element );
		if ( Z_TYPE_PP( paint_element ) != IS_ARRAY || zend_hash_num_elements( Z_ARRVAL_PP( paint_element ) ) < 4 )
		{
			php_error( E_ERROR, "expected an array of for elements for 'paint' element of array" );
			return;
		}
		paint_rect = Z_ARRVAL_PP( paint_element );
		// TODO: error checking
		zend_hash_index_find( paint_rect, 0, ( void ** ) &left );
		zend_hash_index_find( paint_rect, 1, ( void ** ) &top );
		zend_hash_index_find( paint_rect, 2, ( void ** ) &right );
		zend_hash_index_find( paint_rect, 3, ( void ** ) &bottom );
		paint_struct.rcPaint.left = Z_LVAL_PP( left );
		paint_struct.rcPaint.top = Z_LVAL_PP( top );
		paint_struct.rcPaint.right = Z_LVAL_PP( right );
		paint_struct.rcPaint.bottom = Z_LVAL_PP( bottom );

		RETURN_BOOL( EndPaint( window_object->window_handle, &paint_struct ) );
	}
}
/** }}} */

/* No methods for this yet */
static const zend_function_entry wingdi_window_functions[] = {
	PHP_ME( WinGdiWindow, beginPaint, NULL, ZEND_ACC_PUBLIC )
	PHP_ME( WinGdiWindow, endPaint, NULL, ZEND_ACC_PUBLIC )
	{NULL, NULL, NULL}
};

/* ----------------------------------------------------------------
  Win\Gdi\Window Custom Object magic                                                 
------------------------------------------------------------------*/

/* {{{ wingdi_window_construction_wrapper
       wraps around the constructor to make sure parent::__construct is always called  */
static void wingdi_window_construction_wrapper(INTERNAL_FUNCTION_PARAMETERS) {
    zval *this = getThis();
    wingdi_window_object *tobj;
    zend_class_entry *this_ce;
    zend_function *zf;
    zend_fcall_info fci = {0};
    zend_fcall_info_cache fci_cache = {0};
    zval *retval_ptr = NULL;
    unsigned i;
 
    tobj = zend_object_store_get_object(this TSRMLS_CC);
    zf = zend_get_std_object_handlers()->get_constructor(this TSRMLS_CC);
    this_ce = Z_OBJCE_P(this);
 
    fci.size = sizeof(fci);
    fci.function_table = &this_ce->function_table;
    fci.object_ptr = this;
    /* fci.function_name = ; not necessary to bother */
    fci.retval_ptr_ptr = &retval_ptr;
    fci.param_count = ZEND_NUM_ARGS();
    fci.params = emalloc(fci.param_count * sizeof *fci.params);
    /* Or use _zend_get_parameters_array_ex instead of loop: */
    for (i = 0; i < fci.param_count; i++) {
        fci.params[i] = (zval **) (zend_vm_stack_top(TSRMLS_C) - 1 -
            (fci.param_count - i));
    }
    fci.object_ptr = this;
    fci.no_separation = 0;
 
    fci_cache.initialized = 1;
    fci_cache.called_scope = EG(current_execute_data)->called_scope;
    fci_cache.calling_scope = EG(current_execute_data)->current_scope;
    fci_cache.function_handler = zf;
    fci_cache.object_ptr = this;
 
    zend_call_function(&fci, &fci_cache TSRMLS_CC);
    if (!EG(exception) && tobj->is_constructed == 0)
		zend_throw_exception_ex(NULL, 0 TSRMLS_CC,
			"parent::__construct() must be called in %s::__construct()", this_ce->name);
    efree(fci.params);
    zval_ptr_dtor(&retval_ptr);
}
/* }}} */

/* {{{ wingdi_window_get_constructor
       gets the constructor for the class  */
static zend_function *wingdi_window_get_constructor(zval *object TSRMLS_DC)
{
    /* Could always return constr_wrapper_fun, but it's uncessary to call the
     * wrapper if instantiating the superclass */
    if (Z_OBJCE_P(object) == ce_wingdi_window)
        return zend_get_std_object_handlers()->
            get_constructor(object TSRMLS_CC);
    else
        return &wingdi_window_constructor_wrapper;
}

/* {{{ wingdi_window_object_free
       frees up the window handle underneath */
static void wingdi_window_object_free(void *object TSRMLS_DC)
{
	wingdi_window_object *window_object = (wingdi_window_object *)object;

	zend_object_std_dtor(&window_object->std TSRMLS_CC);

	if(window_object->window_handle) {
		// TODO: FREE ME
	}
	
	efree(window_object);
}
/* }}} */

/* {{{ wingdi_window_object_create
       pretty basic item with internal window handle  */
static zend_object_value wingdi_window_object_create(zend_class_entry *ce TSRMLS_DC)
{
    zend_object_value          retval;
    wingdi_window_object       *window_object;
 
	window_object = ecalloc(1, sizeof(wingdi_window_object));
    zend_object_std_init((zend_object *) window_object, ce TSRMLS_CC);
	window_object->window_handle = NULL;
	window_object->is_constructed = FALSE;
 
    zend_hash_copy(window_object->std.properties, &(ce->default_properties),
        (copy_ctor_func_t) zval_add_ref, NULL, sizeof(zval*));
 
    retval.handle = zend_objects_store_put(window_object,
        (zend_objects_store_dtor_t) zend_objects_destroy_object,
        (zend_objects_free_object_storage_t) wingdi_window_object_free,
        NULL TSRMLS_CC);
    retval.handlers = &wingdi_window_object_handlers;
    return retval;
}
/* }}} */

/* ----------------------------------------------------------------
  Win\Gdi\Bitmap LifeCycle Functions                                                    
------------------------------------------------------------------*/

PHP_MINIT_FUNCTION(wingdi_window)
{
	zend_class_entry ce;

	memcpy(&wingdi_window_object_handlers, zend_get_std_object_handlers(),
        sizeof wingdi_window_object_handlers);
    wingdi_window_object_handlers.get_constructor = wingdi_window_get_constructor;

	INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_NS, "Window", wingdi_window_functions);
	ce_wingdi_window = zend_register_internal_class(&ce TSRMLS_CC);
	ce_wingdi_window->create_object = wingdi_window_object_create;
	ce_wingdi_window->ce_flags     |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;

	wingdi_window_constructor_wrapper.type = ZEND_INTERNAL_FUNCTION;
    wingdi_window_constructor_wrapper.common.function_name = "internal_construction_wrapper";
    wingdi_window_constructor_wrapper.common.scope = ce_wingdi_window;
    wingdi_window_constructor_wrapper.common.fn_flags = ZEND_ACC_PROTECTED;
    wingdi_window_constructor_wrapper.common.prototype = NULL;
    wingdi_window_constructor_wrapper.common.required_num_args = 0;
    wingdi_window_constructor_wrapper.common.arg_info = NULL;
    wingdi_window_constructor_wrapper.common.pass_rest_by_reference = 0;
    wingdi_window_constructor_wrapper.common.return_reference = 0;
    wingdi_window_constructor_wrapper.internal_function.handler = wingdi_window_construction_wrapper;
    wingdi_window_constructor_wrapper.internal_function.module = EG(current_module);

	return SUCCESS;
}