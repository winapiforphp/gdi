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
	DEVMODE lpInitData;
  	int drivername_len,
        devicename_len;
	wingdi_devicecontext_object *context_object = (wingdi_devicecontext_object*)zend_objects_get_address(getThis() TSRMLS_CC);
    zval *devmode_array = NULL;

	WINGDI_ERROR_HANDLING();
	/* default is to create a DC for "DISPLAY" */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ssa", 
            &drivername, &drivername_len, &devicename, &devicename_len, &devmode_array) == FAILURE) {
		return;
	}
    WINGDI_RESTORE_ERRORS();

    lpInitData.dmFields = 0;
    if (devmode_array && Z_TYPE_P(devmode_array) == IS_ARRAY)
    {
        HashTable *hash = Z_ARRVAL_P(devmode_array);
        zval **tmp;

        #define DM_HASH_FIND(find) zend_hash_find(hash, find, sizeof(find) - 1, (void **)&tmp)
        #define DM_JUGGLE(juggle) wingdi_juggle_type(*tmp, juggle TSRMLS_CC)
        if (DM_HASH_FIND("devicename") == SUCCESS)
            if (wingdi_juggle_type(*tmp, IS_STRING TSRMLS_CC) == SUCCESS)
                strcpy(lpInitData.dmDeviceName, Z_STRVAL_PP(tmp));

        if (DM_HASH_FIND("specversion") == SUCCESS) {
            if (wingdi_juggle_type(*tmp, IS_LONG TSRMLS_CC) == SUCCESS)
                lpInitData.dmSpecVersion = (WORD)Z_LVAL_PP(tmp);
        }

        if (DM_HASH_FIND("driverversion") == SUCCESS) {
            if (wingdi_juggle_type(*tmp, IS_LONG TSRMLS_CC) == SUCCESS)
                lpInitData.dmDriverVersion = (WORD)Z_LVAL_PP(tmp);
        }

        if (DM_HASH_FIND("orientation") == SUCCESS) {
            if (wingdi_juggle_type(*tmp, IS_LONG TSRMLS_CC) == SUCCESS) {
                lpInitData.dmOrientation = (short)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_ORIENTATION;
            }
        }

        if (DM_HASH_FIND("papersize") == SUCCESS) {
            if (wingdi_juggle_type(*tmp, IS_LONG TSRMLS_CC) == SUCCESS) {
                lpInitData.dmPaperSize = (short)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_PAPERSIZE;
            }
        }

        if (DM_HASH_FIND("paperlength") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmPaperLength = (short)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_PAPERLENGTH;
            }
        }

        if (DM_HASH_FIND("paperwidth") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmPaperWidth = (short)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_PAPERWIDTH;
            }
        }

        if (DM_HASH_FIND("scale") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmScale = (short)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_SCALE;
            }
        }

        if (DM_HASH_FIND("copies") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmCopies = (short)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_COPIES;
            }
        }

        if (DM_HASH_FIND("defaultsource") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmDefaultSource = (short)(tmp);
                lpInitData.dmFields |= DM_DEFAULTSOURCE;
            }
        }

        if (DM_HASH_FIND("printquality") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmPrintQuality = (short)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_PRINTQUALITY;
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
                lpInitData.dmPosition = point;
            } else {
                php_error_docref(NULL TSRMLS_CC, E_ERROR, "expected an array for 'position' index");
                return;
            }
        }

        if (DM_HASH_FIND("displayorientation") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmDisplayOrientation = (DWORD)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_DISPLAYORIENTATION;
            }
        }

        if (DM_HASH_FIND("displayfixedoutput") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmDisplayFixedOutput = (DWORD)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_DISPLAYFIXEDOUTPUT;
            }
        }

        if (DM_HASH_FIND("color") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmColor = (short)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_COLOR;
            }
        }
        if (DM_HASH_FIND("duplex") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmDuplex = (short)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_DUPLEX;
            }
        }
        if (DM_HASH_FIND("yresolution") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmYResolution = (short)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_YRESOLUTION;
            }
        }
        if (DM_HASH_FIND("ttoption") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmTTOption = (short)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_TTOPTION;
            }
        }
        if (DM_HASH_FIND("collate") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmCollate = (short)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_COLLATE;
            }
        }
        if (DM_HASH_FIND("formname") == SUCCESS) {
            if (DM_JUGGLE(IS_STRING) == SUCCESS) {
                strcpy(lpInitData.dmFormName, Z_STRVAL_PP(tmp));
                lpInitData.dmFields |= DM_FORMNAME;
            }
        }
        if (DM_HASH_FIND("logpixels") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmLogPixels = (WORD)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_LOGPIXELS;
            }
        }
        if (DM_HASH_FIND("bitsperpel") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmBitsPerPel = (DWORD)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_BITSPERPEL;
            }
        }
        if (DM_HASH_FIND("pelswidth") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmPelsWidth = (DWORD)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_PELSWIDTH;
            }
        }
        if (DM_HASH_FIND("pelsheight") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmPelsHeight = (DWORD)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_PELSHEIGHT;
            }
        }
        // Getting bored now :(
        if (DM_HASH_FIND("displayflags") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmDisplayFlags = (DWORD)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_DISPLAYFLAGS;
            }
        }
        if (DM_HASH_FIND("nup") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmNup = (DWORD)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_NUP;
            }
        }
        if (DM_HASH_FIND("displayfrequency") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmDisplayFrequency = (DWORD)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_DISPLAYFREQUENCY;
            }
        }
#if (WINVER >= 0x0400)
        if (DM_HASH_FIND("icmmethod") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmICMMethod = (DWORD)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_ICMMETHOD;
            }
        }
        if (DM_HASH_FIND("icmintent") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmICMIntent = (DWORD)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_ICMINTENT;
            }
        }
        if (DM_HASH_FIND("mediatype") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmMediaType = (DWORD)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_MEDIATYPE;
            }
        }
        if (DM_HASH_FIND("dithertype") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmDitherType = (DWORD)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_DITHERTYPE;
            }
        }
        if (DM_HASH_FIND("reserved1") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmReserved1 = (DWORD)Z_LVAL_PP(tmp);
            }
        }
        if (DM_HASH_FIND("reserved2") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmReserved2 = (DWORD)Z_LVAL_PP(tmp);
            }
        }
#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
        if (DM_HASH_FIND("panningwidth") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmPanningWidth = (DWORD)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_PANNINGWIDTH;
            }
        }
        if (DM_HASH_FIND("panningheight") == SUCCESS) {
            if (DM_JUGGLE(IS_LONG) == SUCCESS) {
                lpInitData.dmPanningHeight = (DWORD)Z_LVAL_PP(tmp);
                lpInitData.dmFields |= DM_PANNINGHEIGHT;
            }
        }
#endif
#endif
    }

	context_object->hdc = CreateDC(drivername, devicename, NULL, &lpInitData);
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