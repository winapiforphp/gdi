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

zend_class_entry     *ce_WinGdiDeviceContext;
zend_function        ctor_wrapper_func;
zend_object_handlers wingdi_devicecontext_object_handlers;

/* ----------------------------------------------------------------
  \Win\Gdi\DeviceContext Userland API                                                      
------------------------------------------------------------------*/

static void _populate_devmode_from_hash (DEVMODE * data, HashTable * hash TSRMLS_DC)
{
    zval **tmp;

    #define DM_HASH_FIND(find) zend_hash_find(hash, find, sizeof(find) - 1, (void **)&tmp)
    #define DM_JUGGLE(juggle) wingdi_juggle_type(*tmp, juggle TSRMLS_CC)

    data->dmFields = 0;
    if (DM_HASH_FIND("devicename") == SUCCESS)
        if (wingdi_juggle_type(*tmp, IS_STRING TSRMLS_CC) == SUCCESS)
            strcpy(data->dmDeviceName, Z_STRVAL_PP(tmp));

    if (DM_HASH_FIND("specversion") == SUCCESS) {
        if (wingdi_juggle_type(*tmp, IS_LONG TSRMLS_CC) == SUCCESS)
            data->dmSpecVersion = (WORD)Z_LVAL_PP(tmp);
    }

    if (DM_HASH_FIND("driverversion") == SUCCESS) {
        if (wingdi_juggle_type(*tmp, IS_LONG TSRMLS_CC) == SUCCESS)
            data->dmDriverVersion = (WORD)Z_LVAL_PP(tmp);
    }

    if (DM_HASH_FIND("orientation") == SUCCESS) {
        if (wingdi_juggle_type(*tmp, IS_LONG TSRMLS_CC) == SUCCESS) {
            data->dmOrientation = (short)Z_LVAL_PP(tmp);
            data->dmFields |= DM_ORIENTATION;
        }
    }

    if (DM_HASH_FIND("papersize") == SUCCESS) {
        if (wingdi_juggle_type(*tmp, IS_LONG TSRMLS_CC) == SUCCESS) {
            data->dmPaperSize = (short)Z_LVAL_PP(tmp);
            data->dmFields |= DM_PAPERSIZE;
        }
    }

    if (DM_HASH_FIND("paperlength") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmPaperLength = (short)Z_LVAL_PP(tmp);
            data->dmFields |= DM_PAPERLENGTH;
        }
    }

    if (DM_HASH_FIND("paperwidth") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmPaperWidth = (short)Z_LVAL_PP(tmp);
            data->dmFields |= DM_PAPERWIDTH;
        }
    }

    if (DM_HASH_FIND("scale") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmScale = (short)Z_LVAL_PP(tmp);
            data->dmFields |= DM_SCALE;
        }
    }

    if (DM_HASH_FIND("copies") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmCopies = (short)Z_LVAL_PP(tmp);
            data->dmFields |= DM_COPIES;
        }
    }

    if (DM_HASH_FIND("defaultsource") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmDefaultSource = (short)(tmp);
            data->dmFields |= DM_DEFAULTSOURCE;
        }
    }

    if (DM_HASH_FIND("printquality") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmPrintQuality = (short)Z_LVAL_PP(tmp);
            data->dmFields |= DM_PRINTQUALITY;
        }
    }
        
    if (DM_HASH_FIND("position") == SUCCESS) {
        if (Z_TYPE_PP(tmp) == IS_ARRAY) {
            POINTL point;
            zval **x, **y;
            if (zend_hash_num_elements(Z_ARRVAL_PP(tmp)) != 2) {
                php_error_docref(NULL TSRMLS_CC, E_ERROR, 
                    "expected 2 elements for 'position' array, got %d", 
                    zend_hash_num_elements(Z_ARRVAL_PP(tmp)));
                return;
            }
            zend_hash_index_find(Z_ARRVAL_PP(tmp), 0, (void **)&x);
            zend_hash_index_find(Z_ARRVAL_PP(tmp), 1, (void **)&y);
            if (Z_TYPE_PP(x) != IS_LONG) convert_to_long(*x);
            if (Z_TYPE_PP(y) != IS_LONG) convert_to_long(*y);
            point.x = Z_LVAL_PP(x);
            point.y = Z_LVAL_PP(y);
            data->dmPosition = point;
        } else {
            php_error_docref(NULL TSRMLS_CC, E_ERROR, "expected an array for 'position' index");
            return;
        }
    }

    if (DM_HASH_FIND("displayorientation") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmDisplayOrientation = (DWORD)Z_LVAL_PP(tmp);
            data->dmFields |= DM_DISPLAYORIENTATION;
        }
    }

    if (DM_HASH_FIND("displayfixedoutput") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmDisplayFixedOutput = (DWORD)Z_LVAL_PP(tmp);
            data->dmFields |= DM_DISPLAYFIXEDOUTPUT;
        }
    }

    if (DM_HASH_FIND("color") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmColor = (short)Z_LVAL_PP(tmp);
            data->dmFields |= DM_COLOR;
        }
    }
    if (DM_HASH_FIND("duplex") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmDuplex = (short)Z_LVAL_PP(tmp);
            data->dmFields |= DM_DUPLEX;
        }
    }
    if (DM_HASH_FIND("yresolution") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmYResolution = (short)Z_LVAL_PP(tmp);
            data->dmFields |= DM_YRESOLUTION;
        }
    }
    if (DM_HASH_FIND("ttoption") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmTTOption = (short)Z_LVAL_PP(tmp);
            data->dmFields |= DM_TTOPTION;
        }
    }
    if (DM_HASH_FIND("collate") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmCollate = (short)Z_LVAL_PP(tmp);
            data->dmFields |= DM_COLLATE;
        }
    }
    if (DM_HASH_FIND("formname") == SUCCESS) {
        if (DM_JUGGLE(IS_STRING) == SUCCESS) {
            strcpy(data->dmFormName, Z_STRVAL_PP(tmp));
            data->dmFields |= DM_FORMNAME;
        }
    }
    if (DM_HASH_FIND("logpixels") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmLogPixels = (WORD)Z_LVAL_PP(tmp);
            data->dmFields |= DM_LOGPIXELS;
        }
    }
    if (DM_HASH_FIND("bitsperpel") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmBitsPerPel = (DWORD)Z_LVAL_PP(tmp);
            data->dmFields |= DM_BITSPERPEL;
        }
    }
    if (DM_HASH_FIND("pelswidth") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmPelsWidth = (DWORD)Z_LVAL_PP(tmp);
            data->dmFields |= DM_PELSWIDTH;
        }
    }
    if (DM_HASH_FIND("pelsheight") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmPelsHeight = (DWORD)Z_LVAL_PP(tmp);
            data->dmFields |= DM_PELSHEIGHT;
        }
    }
    // Getting bored now :(
    if (DM_HASH_FIND("displayflags") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmDisplayFlags = (DWORD)Z_LVAL_PP(tmp);
            data->dmFields |= DM_DISPLAYFLAGS;
        }
    }
    if (DM_HASH_FIND("nup") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmNup = (DWORD)Z_LVAL_PP(tmp);
            data->dmFields |= DM_NUP;
        }
    }
    if (DM_HASH_FIND("displayfrequency") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmDisplayFrequency = (DWORD)Z_LVAL_PP(tmp);
            data->dmFields |= DM_DISPLAYFREQUENCY;
        }
    }
#if (WINVER >= 0x0400)
    if (DM_HASH_FIND("icmmethod") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmICMMethod = (DWORD)Z_LVAL_PP(tmp);
            data->dmFields |= DM_ICMMETHOD;
        }
    }
    if (DM_HASH_FIND("icmintent") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmICMIntent = (DWORD)Z_LVAL_PP(tmp);
            data->dmFields |= DM_ICMINTENT;
        }
    }
    if (DM_HASH_FIND("mediatype") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmMediaType = (DWORD)Z_LVAL_PP(tmp);
            data->dmFields |= DM_MEDIATYPE;
        }
    }
    if (DM_HASH_FIND("dithertype") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmDitherType = (DWORD)Z_LVAL_PP(tmp);
            data->dmFields |= DM_DITHERTYPE;
        }
    }
    if (DM_HASH_FIND("reserved1") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmReserved1 = (DWORD)Z_LVAL_PP(tmp);
        }
    }
    if (DM_HASH_FIND("reserved2") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmReserved2 = (DWORD)Z_LVAL_PP(tmp);
        }
    }
#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
    if (DM_HASH_FIND("panningwidth") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmPanningWidth = (DWORD)Z_LVAL_PP(tmp);
            data->dmFields |= DM_PANNINGWIDTH;
        }
    }
    if (DM_HASH_FIND("panningheight") == SUCCESS) {
        if (DM_JUGGLE(IS_LONG) == SUCCESS) {
            data->dmPanningHeight = (DWORD)Z_LVAL_PP(tmp);
            data->dmFields |= DM_PANNINGHEIGHT;
        }
    }
#endif
#endif
}


/* {{{ proto int \Win\Gdi\DeviceContext->construct([string devicename])
       Creates a new display context.

	   Returns true on success, false on failure
 */
PHP_METHOD(WinGdiDeviceContext, __construct)
{
	LPCTSTR drivername = "DISPLAY";
	LPCTSTR devicename = NULL;
	DEVMODE lpInitData;
  	int drivername_len,
        devicename_len;
	wingdi_devicecontext_object *context_object = 
        (wingdi_devicecontext_object*)zend_objects_get_address(getThis() TSRMLS_CC);
    zval *devmode_array = NULL;

	WINGDI_ERROR_HANDLING();
	/* default is to create a DC for "DISPLAY" */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ssa", 
            &drivername, &drivername_len, &devicename, &devicename_len, &devmode_array) == FAILURE) {
		return;
	}
    WINGDI_RESTORE_ERRORS();

    if (devmode_array && Z_TYPE_P(devmode_array) == IS_ARRAY)
    {
        _populate_devmode_from_hash(&lpInitData, Z_ARRVAL_P(devmode_array) TSRMLS_CC);
    }

	context_object->hdc = CreateDC(drivername, devicename, NULL, &lpInitData);
	if (context_object->hdc == NULL) {
		zend_throw_exception(ce_wingdi_exception, "Error creating device context", 0 TSRMLS_CC);
        return;
	}

    context_object->constructed = 1;
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
        display_object->constructed = 1;
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
	wingdi_devicecontext_object *context_object = 
        (wingdi_devicecontext_object*)zend_object_store_get_object(object TSRMLS_CC);

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
	wingdi_devicecontext_object *context_object = 
        (wingdi_devicecontext_object*)zend_object_store_get_object(object TSRMLS_CC);

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
	wingdi_devicecontext_object *context_object = 
        (wingdi_devicecontext_object*)zend_object_store_get_object(object TSRMLS_CC);

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

PHP_METHOD(WinGdiDeviceContext, getDisplayDevice)
{
    DISPLAY_DEVICE display_device_data;
    LPCTSTR device_name = NULL;
    DWORD device_num = 0,
          flags = 0;
    BOOL result;
    zval *z_device;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &z_device, &flags) != SUCCESS)
        return;
    WINGDI_RESTORE_ERRORS();

    // Figure out whether we have a number or string for first param
    if (Z_TYPE_P(z_device) == IS_STRING)
        device_name = (LPCTSTR)Z_STRVAL_P(z_device);
    else if (Z_TYPE_P(z_device) == IS_LONG)
        device_num = (DWORD)Z_LVAL_P(z_device);
    else {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "expected int or string for param 1, got %s",
            zend_zval_type_name(z_device));
        return;
    }
    flags = EDD_GET_DEVICE_INTERFACE_NAME;

    display_device_data.cb = sizeof(DISPLAY_DEVICE);
    result = EnumDisplayDevices(device_name, device_num, &display_device_data, flags);
    if (!result) {
        RETURN_FALSE;
    }
    // Populate return with data
    array_init(return_value);
    add_assoc_string(return_value, "devicename", display_device_data.DeviceName, 1);
    add_assoc_string(return_value, "devicestring", display_device_data.DeviceString, 1);
    add_assoc_string(return_value, "deviceid", display_device_data.DeviceID, 1);
    add_assoc_string(return_value, "devicekey", display_device_data.DeviceKey, 1);
    add_assoc_long(return_value, "stateflags", display_device_data.StateFlags);
}

PHP_METHOD(WinGdiDeviceContext, changeDisplaySettings)
{
    DEVMODE data;
    DWORD flags;
    zval *devmode_array;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "al", &devmode_array, &flags) != SUCCESS)
        return;
    WINGDI_RESTORE_ERRORS();

    _populate_devmode_from_hash(&data, Z_ARRVAL_P(devmode_array) TSRMLS_CC);
    RETURN_LONG(ChangeDisplaySettings(&data, flags));
}

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
    PHP_ME(WinGdiDeviceContext, getDisplayDevice, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(WinGdiDeviceContext, changeDisplaySettings, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
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
    object->constructed = 0;

	ALLOC_HASHTABLE(object->std.properties);
	zend_hash_init(object->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
	zend_hash_copy(object->std.properties, &ce->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));

	retval.handle   = zend_objects_store_put(object, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) wingdi_devicecontext_free_storage, NULL TSRMLS_CC);
	object->handle  = retval.handle;
	retval.handlers = &wingdi_devicecontext_object_handlers;
	return retval;
}
/* }}} */

static zend_function * get_constructor (zval * object TSRMLS_DC)
{
    if (Z_OBJCE_P(object) == ce_WinGdiDeviceContext)
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

/* ----------------------------------------------------------------
  \Win\Gdi\DeviceContext LifeCycle Functions                                                    
------------------------------------------------------------------*/

/* {{{ PHP_MINIT_FUNCTION(wingdi_devicecontext)
	Registers the \Win\Gdi\DeviceContext class
 */
PHP_MINIT_FUNCTION(wingdi_devicecontext)
{
	zend_class_entry ce;
	
	INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_NS, "DeviceContext", wingdi_devicecontext_functions);
	ce_WinGdiDeviceContext = zend_register_internal_class(&ce TSRMLS_CC);
	ce_WinGdiDeviceContext->create_object = wingdi_devicecontext_object_new;

    memcpy(&wingdi_devicecontext_object_handlers, zend_get_std_object_handlers(),
        sizeof(wingdi_devicecontext_object_handlers));
    wingdi_devicecontext_object_handlers.get_constructor = get_constructor;

    ctor_wrapper_func.type                 = ZEND_INTERNAL_FUNCTION;
    ctor_wrapper_func.common.function_name = "internal_construction_wrapper";
    ctor_wrapper_func.common.scope         = ce_WinGdiDeviceContext;
    ctor_wrapper_func.common.fn_flags      = ZEND_ACC_PROTECTED;
    ctor_wrapper_func.common.prototype     = NULL;
    ctor_wrapper_func.common.required_num_args = 0;
    ctor_wrapper_func.common.arg_info      = NULL;
    ctor_wrapper_func.internal_function.handler = construction_wrapper;
    ctor_wrapper_func.internal_function.module  = EG(current_module);

	//zend_declare_property_long(ce_WinGdiColor, "red", sizeof("red") - 1, 0, ZEND_ACC_PUBLIC TSRMLS_CC);
	//zend_declare_property_long(ce_WinGdiColor, "green", sizeof("green") - 1, 0, ZEND_ACC_PUBLIC TSRMLS_CC);
	//zend_declare_property_long(ce_WinGdiColor, "blue", sizeof("blue") - 1, 0, ZEND_ACC_PUBLIC TSRMLS_CC);
	//zend_declare_property_long(ce_WinGdiColor, "hex", sizeof("hex") - 1, 0, ZEND_ACC_PUBLIC TSRMLS_CC);

	return SUCCESS;
}
/* }}} */