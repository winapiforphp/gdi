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
  |         Mark G. Skilbeck   <markskilbeck@php.net>                    |
  +----------------------------------------------------------------------+
*/

/* $Id$ */ 

#include "php_wingdi.h"
#include "zend_exceptions.h" 

zend_class_entry *ce_WinGdiDeviceContext; 
zend_object_handlers wingdi_devicecontext_object_handlers;

/* ----------------------------------------------------------------
  \Win\Gdi\DeviceContext Userland API                                                      
------------------------------------------------------------------*/

/* {{{ proto int \Win\Gdi\DeviceContext->construct([string devicename])
       Creates a new display context.

	   Returns true on success, false on failure
 */
PHP_METHOD(WinGdiDeviceContext, __construct)
{
	LPCTSTR drivername = "DISPLAY";
	LPCTSTR devicename = NULL;
	//const DEVMODE *lpInitData - todo - allow settings
	int drivername_len;

	zval *object = getThis();
	wingdi_devicecontext_object *context_object = (wingdi_devicecontext_object*)zend_objects_get_address(object TSRMLS_CC);

	WINGDI_ERROR_HANDLING()
	/* default is to create a DC for "DISPLAY" */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &drivername, &drivername_len) == FAILURE) {
		return;
	}
    WINGDI_RESTORE_ERRORS()

	/* TODO: support DEVMODE struct as settings array */
	context_object->hdc = CreateDC(drivername, devicename, NULL, NULL);
	if (context_object->hdc == NULL) {
		zend_throw_exception(ce_wingdi_exception, "Error creating device context", 0 TSRMLS_CC);
	}
}
/* }}} */

/* {{{ proto \Win\Gdi\DeviceContext object \Win\Gdi\DeviceContext::get([object window])
       Retrieves the display context for a specified window.

	   This is basically a static constructor for DeviceContext that retrieves an existing
	   DC instead of creating a new one from scratch.
 */
PHP_METHOD(WinGdiDeviceContext, get)
{
	HDC hdc = NULL;
	HWND window_handle = NULL;
	HRGN clip_region = NULL;
	DWORD flags = 0;
	wingdi_devicecontext_object *display_object;

	WINGDI_ERROR_HANDLING()

	/* Passing no parameters or null is fine, you get the desktop, otherwise get a window handle */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

	/* TODO: support parameters */
	hdc = GetDCEx(window_handle, clip_region, flags);
	if (hdc == NULL) {
		zend_throw_exception(ce_wingdi_argexception, "Error retrieving device context", 0 TSRMLS_CC);
	} else {
		/* Make our object a pretty return value */
		object_init_ex(return_value, ce_WinGdiDeviceContext); 
		Z_SET_REFCOUNT_P(return_value, 1);
		Z_UNSET_ISREF_P(return_value); 
		display_object = (wingdi_devicecontext_object*)zend_objects_get_address(return_value TSRMLS_CC);
		display_object->hdc = hdc;
	}
	WINGDI_RESTORE_ERRORS()
}
/* }}} */

/* {{{ proto int \Win\Gdi\DeviceContext->cancel()
       Cancels any pending operation on the specified device context.

	   Returns true on success, false on failure
 */
PHP_METHOD(WinGdiDeviceContext, cancel)
{
	zval *object = getThis();
	wingdi_devicecontext_object *context_object = (wingdi_devicecontext_object*)wingdi_devicecontext_object_get(object TSRMLS_CC);

	if (zend_parse_parameters_none() == FAILURE) {
		return;
    }

	RETURN_BOOL(CancelDC(context_object->hdc));
}
/* }}} */

/* {{{ proto int \Win\Gdi\DeviceContext->save()
       Saves the current state of the device context by copying
	   selected objects and graphic modes to a context stack.

	   Returns false on failure, or an integer of the save location
	   on the stack on success.
 */
PHP_METHOD(WinGdiDeviceContext, save)
{
	int worked;
	zval *object = getThis();
	wingdi_devicecontext_object *context_object = (wingdi_devicecontext_object*)wingdi_devicecontext_object_get(object TSRMLS_CC);

	if (zend_parse_parameters_none() == FAILURE) {
		return;
    }

	worked = SaveDC(context_object->hdc);

	if (worked == 0) {
		wingdi_create_error(GetLastError(), ce_wingdi_exception TSRMLS_CC);
	}
	RETURN_LONG(worked);
}
/* }}} */

/* {{{ proto int \Win\Gdi\DeviceContext->restore([int savedstate])
       Restores a device context to the specified state.

	   If no parameter is provided, the last state will be "popped off" the stack.
	   If a negative parameter is provided, that number of states will be popped off.
	   If a positive number is provided, it is expected to be one of the numbers provided
	   by a call to DeviceContext->save(); and all saves before that number will
	   be popped off.

	   Returns true on success, false on failure
 */
PHP_METHOD(WinGdiDeviceContext, restore)
{
	long savedstate = -1;
	zval *object = getThis();
	wingdi_devicecontext_object *context_object = (wingdi_devicecontext_object*)wingdi_devicecontext_object_get(object TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &savedstate) == FAILURE) {
		return;
	}

	/* A bit of validation, we're going long -> int */
	if (savedstate > INT_MAX || savedstate < INT_MIN) {
		 php_error_docref(NULL TSRMLS_CC, E_WARNING, "Savedstate must be less than %d or greater than %d", INT_MAX, INT_MIN);
		 savedstate = -1;
	}

	RETURN_BOOL(RestoreDC(context_object->hdc, savedstate));
}
/* }}} */

/* {{{ Method definitions */
ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_devicecontext___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, devicename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_devicecontext_restore, 0, 0, 0)
	ZEND_ARG_INFO(0, savedstate)
ZEND_END_ARG_INFO()

static const zend_function_entry wingdi_devicecontext_functions[] = {
	PHP_ME(WinGdiDeviceContext, __construct, arginfo_wingdi_devicecontext___construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(WinGdiDeviceContext, get, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(WinGdiDeviceContext, cancel, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(WinGdiDeviceContext, save, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(WinGdiDeviceContext, restore, arginfo_wingdi_devicecontext_restore, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};
/* }}} */

/* ----------------------------------------------------------------
  \Win\Gdi\DeviceContext Custom Object handlers                                                 
------------------------------------------------------------------*/

/* {{{ wingdi_devicecontext_free_storage
       if the hdc was done with CreateDC then DeleteDC is used
	   if the hdc was done with GetDC then ReleaseDC is used */
void wingdi_devicecontext_free_storage(wingdi_devicecontext_object *intern TSRMLS_DC)
{
	if(intern->hdc && intern->window_handle) {
		ReleaseDC(intern->window_handle, intern->hdc);
	} else if (intern->hdc) {
		DeleteDC(intern->hdc);
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

/* }}} */

/* {{{ wingdi_devicecontext_object_new
       creates a new wingdi_devicecontext object and takes care of the internal junk */
zend_object_value wingdi_devicecontext_object_new(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value retval;
	wingdi_devicecontext_object *object;
	zval *tmp;

	object = emalloc(sizeof(wingdi_devicecontext_object));
	object->std.ce = ce;
	object->std.guards = NULL;
	object->hdc = NULL;
	object->window_handle = NULL;

	ALLOC_HASHTABLE(object->std.properties);
	zend_hash_init(object->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
	zend_hash_copy(object->std.properties, &ce->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));

	retval.handle = zend_objects_store_put(object, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) wingdi_devicecontext_free_storage, NULL TSRMLS_CC);
	object->handle = retval.handle;
	retval.handlers = &wingdi_devicecontext_object_handlers;
	return retval;
}
/* }}} */

/* ----------------------------------------------------------------
  \Win\Gdi\DeviceContext LifeCycle Functions                                                    
------------------------------------------------------------------*/

/* {{{ PHP_MINIT_FUNCTION(wingdi_devicecontext)
	Registers the \Win\Gdi\DeviceContext class
 */
PHP_MINIT_FUNCTION(wingdi_devicecontext)
{
	zend_class_entry ce;
	memcpy(&wingdi_devicecontext_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_NS, "DeviceContext", wingdi_devicecontext_functions);
	ce_WinGdiDeviceContext = zend_register_internal_class(&ce TSRMLS_CC);
	ce_WinGdiDeviceContext->create_object  = wingdi_devicecontext_object_new;

	//zend_declare_property_long(ce_WinGdiColor, "red", sizeof("red") - 1, 0, ZEND_ACC_PUBLIC TSRMLS_CC);
	//zend_declare_property_long(ce_WinGdiColor, "green", sizeof("green") - 1, 0, ZEND_ACC_PUBLIC TSRMLS_CC);
	//zend_declare_property_long(ce_WinGdiColor, "blue", sizeof("blue") - 1, 0, ZEND_ACC_PUBLIC TSRMLS_CC);
	//zend_declare_property_long(ce_WinGdiColor, "hex", sizeof("hex") - 1, 0, ZEND_ACC_PUBLIC TSRMLS_CC);

	return SUCCESS;
}
/* }}} */