/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 2007-2008 Elizabeth M. Smith, Sara Golemon, Tom Rogers |
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
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#include "php_wingdi.h"
#include <zend_exceptions.h>

/* Custom Object junk */
static zend_class_entry     *ce_wingdi_brush;
static zend_object_handlers wingdibrush_object_handlers;
static zend_function        ctor_wrapper_func;

void wingdibrush_write_property(zval *object, zval *member, zval *value TSRMLS_DC);
zval *wingdibrush_read_property(zval *object, zval *member, int type TSRMLS_DC);

void wingdibrush_free_storage(wingdi_brush_object *intern TSRMLS_DC)
{
	if(intern->brush_handle && !intern->system_brush) {
		DeleteObject(intern->brush_handle);
	}
	if (intern->std.guards) {
		zend_hash_destroy(intern->std.guards);
		FREE_HASHTABLE(intern->std.guards);		
	}
	
	if (intern->std.properties) {
		zend_hash_destroy(intern->std.properties);
		FREE_HASHTABLE(intern->std.properties);
	}
	efree(intern);
}

zend_object_value wingdibrush_new(zend_class_entry *ce TSRMLS_DC)
{
	wingdi_brush_object *object;
	zend_object_value   retval;
	zval tmp;
	
	object = emalloc(sizeof(wingdi_brush_object));
	object->std.ce       = ce;
	object->std.guards   = NULL;
	object->brush_handle = NULL;
	object->prop_handler = NULL;
    object->is_constructed  = 0;
	
	ALLOC_HASHTABLE(object->std.properties);
	zend_hash_init(object->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
	zend_hash_copy(object->std.properties, 
        &ce->default_properties, 
        (copy_ctor_func_t) zval_add_ref, 
        (void *) &tmp, 
        sizeof(zval *));

	retval.handle   = zend_objects_store_put(object, 
        (zend_objects_store_dtor_t)zend_objects_destroy_object, 
        (zend_objects_free_object_storage_t) wingdibrush_free_storage, 
        NULL TSRMLS_CC);
	retval.handlers = &wingdibrush_object_handlers;

	return retval;
}

static zend_function * get_constructor (zval * object TSRMLS_DC)
{
    if (Z_OBJCE_P(object) == ce_wingdi_brush)
    {
        return zend_get_std_object_handlers()->get_constructor(object TSRMLS_CC);
    }
    else
    {
        return &ctor_wrapper_func;
    }
}

static void construction_wrapper (INTERNAL_FUNCTION_PARAMETERS)
{
    zend_fcall_info_cache fci_cache = {0};
    zend_fcall_info       fci       = {0};
    zend_class_entry      *this_ce;
    zend_function         *zf;
    wingdi_path_object    *path_obj;
    zval                  *_this       = getThis(),
                          *retval_ptr = NULL;

    path_obj = zend_object_store_get_object(_this TSRMLS_CC);
    zf       = zend_get_std_object_handlers()->get_constructor(_this TSRMLS_CC);
    this_ce  = Z_OBJCE_P(_this);

    fci.size           = sizeof(fci);
    fci.function_table = &this_ce->function_table;
    fci.retval_ptr_ptr = &retval_ptr;
    fci.object_ptr     = _this;
    fci.param_count    = ZEND_NUM_ARGS();
    fci.params         = emalloc(fci.param_count * sizeof *fci.params);
    fci.no_separation  = 0;
    _zend_get_parameters_array_ex(fci.param_count, fci.params TSRMLS_CC);

    fci_cache.initialized      = 1;
    fci_cache.called_scope     = EG(current_execute_data)->called_scope;
    fci_cache.calling_scope    = EG(current_execute_data)->current_scope;
    fci_cache.function_handler = zf;
    fci_cache.object_ptr       = _this;

    zend_call_function(&fci, &fci_cache TSRMLS_CC);

    if (!EG(exception) && path_obj->constructed == 0)
        zend_throw_exception_ex(NULL, 0 TSRMLS_CC,
            "parent::__construct() must be called in %s::__construct()", this_ce->name);

    efree(fci.params);
    zval_ptr_dtor(&retval_ptr);
}

/* Methods for the class */
PHP_METHOD(WinGdiBrush, __construct);

/* arginfo for the class */
ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdibrush___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, style)
	ZEND_ARG_INFO(0, color)
	ZEND_ARG_INFO(0, hatch)
	//ZEND_ARG_OBJ_INFO(0, pattern, Win::Gdi::PackedDib, 0)
ZEND_END_ARG_INFO();

/* regular Window methods */
static const zend_function_entry wingdibrush_functions[] = {
	PHP_ME(WinGdiBrush, __construct, arginfo_wingdibrush___construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	{NULL, NULL, NULL}
};

/* {{{ __construct */
PHP_METHOD(WinGdiBrush, __construct)
{
	UINT style = BS_SOLID;
	int index = 0;
	LONG hstyle = 0;
	COLORREF colref = 0;
	zval *color = NULL, *hatch = NULL;
	LOGBRUSH lb;
	wingdi_brush_object *brush_obj = (wingdi_brush_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
	/* default is to create a solid black brush */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|lzz", &style, &color, &hatch) == FAILURE) {
		return;
	}

	if(style == -1) {
		brush_obj->system_brush = TRUE;
		if(color) {
			convert_to_long(color);
			index = Z_LVAL_P(color);
		}
		brush_obj->brush_handle = GetSysColorBrush(index);
	} else {
		
		brush_obj->system_brush = FALSE;
		
		if(color && (Z_TYPE_P(color) == IS_OBJECT)) {
			
			zval *colval;
			
			MAKE_STD_ZVAL(colval);
			ZVAL_STRING(colval, "hex", 0);
			colref = Z_LVAL_P(std_object_handlers.read_property(color, colval, IS_LONG TSRMLS_CC));
		}
		
		memset(&lb, 0, sizeof(LOGBRUSH));
		
		lb.lbStyle = style;
		lb.lbColor = colref;
		lb.lbColor = hstyle;
		brush_obj->brush_handle = CreateBrushIndirect(&lb);
	}

	if (brush_obj->brush_handle) {
		zend_update_property_bool(ce_wingdi_brush, getThis(), "system", sizeof("system") - 1, (long)brush_obj->system_brush TSRMLS_CC);
	}

    brush_obj->is_constructed = 1;
	return;
}
/* }}} */

PHP_MINIT_FUNCTION(wingdi_brush)
{
	zend_class_entry ce;
	
    INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_NS, "Brush", wingdibrush_functions);
	ce_wingdi_brush = zend_register_internal_class(&ce TSRMLS_CC);
	ce_wingdi_brush->create_object = wingdibrush_new;

    memcpy(&wingdibrush_object_handlers,
        zend_get_std_object_handlers(), 
        sizeof(zend_object_handlers));
    wingdibrush_object_handlers.get_constructor = get_constructor;

    ctor_wrapper_func.type                 = ZEND_INTERNAL_FUNCTION;
    ctor_wrapper_func.common.function_name = "internal_construction_wrapper";
    ctor_wrapper_func.common.scope         = ce_wingdi_brush;
    ctor_wrapper_func.common.fn_flags      = ZEND_ACC_PROTECTED;
    ctor_wrapper_func.common.prototype     = NULL;
    ctor_wrapper_func.common.required_num_args = 0;
    ctor_wrapper_func.common.arg_info      = NULL;
    ctor_wrapper_func.internal_function.handler = construction_wrapper;
    ctor_wrapper_func.internal_function.module  = EG(current_module);

	zend_declare_property_long(ce_wingdi_brush, "handle", sizeof("handle") - 1, 0, ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_bool(ce_wingdi_brush, "system", sizeof("system") - 1, FALSE, ZEND_ACC_PUBLIC TSRMLS_CC);
	/* Constants -  Brush styles*/
	zend_declare_class_constant_long(ce_wingdi_brush, "DIBPATTERN", sizeof("DIBPATTERN") - 1, BS_DIBPATTERN TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "DIBPATTERN8X8", sizeof("DIBPATTERN8X8") - 1, BS_DIBPATTERN8X8 TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "DIBPATTERNPT", sizeof("DIBPATTERNPT") - 1, BS_DIBPATTERNPT TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "HATCHED", sizeof("HATCHED") - 1, BS_HATCHED TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "HOLLOW", sizeof("HOLLOW") - 1, BS_HOLLOW TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "NULL", sizeof("NULL") - 1, BS_NULL TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "PATTERN", sizeof("PATTERN") - 1, BS_PATTERN TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "PATTERN8X8", sizeof("PATTERN8X8") - 1, BS_PATTERN8X8 TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "SOLID", sizeof("SOLID") - 1, BS_SOLID TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "SYSTEMBRUSH", sizeof("SYSTEMBRUSH") - 1, -1 TSRMLS_CC);
	/* Pallette type */
	zend_declare_class_constant_long(ce_wingdi_brush, "DIB_PAL_COLORS", sizeof("DIB_PAL_COLORS") - 1, DIB_PAL_COLORS TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "DIB_RGB_COLORS", sizeof("DIB_RGB_COLORS") - 1, DIB_RGB_COLORS TSRMLS_CC);
	/* Hatch styles */
	zend_declare_class_constant_long(ce_wingdi_brush, "BDIAGONAL", sizeof("BDIAGONAL") - 1, HS_BDIAGONAL TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "CROSS", sizeof("CROSS") - 1, HS_CROSS TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "DIAGCROSS", sizeof("DIAGCROSS") - 1, HS_DIAGCROSS TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "FDIAGONAL", sizeof("FDIAGONAL") - 1, HS_FDIAGONAL TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "HORIZONTAL", sizeof("HORIZONTAL") - 1, HS_HORIZONTAL TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "VERTICAL", sizeof("VERTICAL") - 1, HS_VERTICAL TSRMLS_CC);
	
	/* Constants -  System Brush colors*/
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_SCROLLBAR", sizeof("COLOR_SCROLLBAR") - 1, COLOR_SCROLLBAR TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_BACKGROUND", sizeof("COLOR_BACKGROUND") - 1, COLOR_BACKGROUND TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_ACTIVECAPTION", sizeof("COLOR_ACTIVECAPTION") - 1, COLOR_ACTIVECAPTION TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_INACTIVECAPTION", sizeof("COLOR_INACTIVECAPTION") - 1, COLOR_INACTIVECAPTION TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_MENU", sizeof("COLOR_MENU") - 1, COLOR_MENU TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_WINDOW", sizeof("COLOR_WINDOW") - 1, COLOR_WINDOW TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_WINDOWFRAME", sizeof("COLOR_WINDOWFRAME") - 1, COLOR_WINDOWFRAME TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_MENUTEXT", sizeof("COLOR_MENUTEXT") - 1, COLOR_MENUTEXT TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_WINDOWTEXT", sizeof("COLOR_WINDOWTEXT") - 1, COLOR_WINDOWTEXT TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_CAPTIONTEXT", sizeof("COLOR_CAPTIONTEXT") - 1, COLOR_CAPTIONTEXT TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_ACTIVEBORDER", sizeof("COLOR_ACTIVEBORDER") - 1, COLOR_ACTIVEBORDER TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_INACTIVEBORDER", sizeof("COLOR_INACTIVEBORDER") - 1, COLOR_INACTIVEBORDER TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_APPWORKSPACE", sizeof("COLOR_APPWORKSPACE") - 1, COLOR_APPWORKSPACE TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_HIGHLIGHT", sizeof("COLOR_HIGHLIGHT") - 1, COLOR_HIGHLIGHT TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_HIGHLIGHTTEXT", sizeof("COLOR_HIGHLIGHTTEXT") - 1, COLOR_HIGHLIGHTTEXT TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_BTNFACE", sizeof("COLOR_BTNFACE") - 1, COLOR_BTNFACE TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_BTNSHADOW", sizeof("COLOR_BTNSHADOW") - 1, COLOR_BTNSHADOW TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_GRAYTEXT", sizeof("COLOR_GRAYTEXT") - 1, COLOR_GRAYTEXT TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_BTNTEXT", sizeof("COLOR_BTNTEXT") - 1, COLOR_BTNTEXT TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_INACTIVECAPTIONTEXT", sizeof("COLOR_INACTIVECAPTIONTEXT") - 1, COLOR_INACTIVECAPTIONTEXT TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_BTNHIGHLIGHT", sizeof("COLOR_BTNHIGHLIGHT") - 1, COLOR_BTNHIGHLIGHT TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_3DDKSHADOW", sizeof("COLOR_3DDKSHADOW") - 1, COLOR_3DDKSHADOW TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_3DLIGHT", sizeof("COLOR_3DLIGHT") - 1, COLOR_3DLIGHT TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_INFOTEXT", sizeof("COLOR_INFOTEXT") - 1, COLOR_INFOTEXT TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_INFOBK", sizeof("COLOR_INFOBK") - 1, COLOR_INFOBK TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_HOTLIGHT", sizeof("COLOR_HOTLIGHT") - 1, COLOR_HOTLIGHT TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_GRADIENTACTIVECAPTION", sizeof("COLOR_GRADIENTACTIVECAPTION") - 1, COLOR_GRADIENTACTIVECAPTION TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_GRADIENTINACTIVECAPTION", sizeof("COLOR_GRADIENTINACTIVECAPTION") - 1, COLOR_GRADIENTINACTIVECAPTION TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_MENUHILIGHT", sizeof("COLOR_MENUHILIGHT") - 1, COLOR_MENUHILIGHT TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_MENUBAR", sizeof("COLOR_MENUBAR") - 1, COLOR_MENUBAR TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_DESKTOP", sizeof("COLOR_DESKTOP") - 1, COLOR_DESKTOP TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_3DFACE", sizeof("COLOR_3DFACE") - 1, COLOR_3DFACE TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_3DSHADOW", sizeof("COLOR_3DSHADOW") - 1, COLOR_3DSHADOW TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_3DHIGHLIGHT", sizeof("COLOR_3DHIGHLIGHT") - 1, COLOR_3DHIGHLIGHT TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_3DHILIGHT", sizeof("COLOR_3DHILIGHT") - 1, COLOR_3DHILIGHT TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_brush, "COLOR_BTNHILIGHT", sizeof("COLOR_BTNHILIGHT") - 1, COLOR_BTNHILIGHT TSRMLS_CC);
	
	return SUCCESS;
}