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
  |         Mark G. Skilbeck   <markskilbeck@php.net>                    |
  +----------------------------------------------------------------------+
*/

/* $Id$ */ 

#include "php_wingdi.h"
#include "zend_exceptions.h" 

zend_class_entry     *ce_wingdi_devicecontext;
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

/* {{{ proto \Win\Gdi\DeviceContext object \Win\Gdi\DeviceContext::get([Win\Gdi\Window window[, Win\Gdi\Region region[, int flags]]])
       Retrieves the display context for a specified window. Can accept a clipping region and flags.

	   This is basically a static constructor for DeviceContext that retrieves an existing
	   DC instead of creating a new one from scratch.
 */
PHP_METHOD(WinGdiDeviceContext, get)
{
	HDC hdc            = NULL;
	HWND window_handle = NULL;
	HRGN clip_region   = NULL;
	DWORD flags        = 0;
	wingdi_devicecontext_object *display_object;
	wingdi_window_object        *window_object;
	wingdi_region_object        *region_object;
	zval                        *region_object_zval = NULL, 
	                            *window_object_zval = NULL;

	WINGDI_ERROR_HANDLING()

	/* Passing no parameters or null is fine, you get the desktop, otherwise get a window handle */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|OOl", &window_object_zval, ce_wingdi_window, &region_object_zval, ce_wingdi_region, &flags) == FAILURE) {
		return;
	}

	if (window_object_zval)
	{
		window_object = (wingdi_window_object *) zend_object_store_get_object(window_object_zval TSRMLS_CC);
		window_handle = window_object->window_handle;
	}
	if (region_object_zval)
	{
		region_object = (wingdi_region_object *) zend_object_store_get_object(region_object_zval TSRMLS_CC);
		clip_region   = region_object->region_handle;
	}

	hdc = GetDCEx(window_handle, clip_region, DCX_CACHE);
	if (hdc == NULL) {
		zend_throw_exception(ce_wingdi_argexception, "Error retrieving device cont	ext", 0 TSRMLS_CC);
	} else {
		/* Make our object a pretty return value */
		object_init_ex(return_value, ce_wingdi_devicecontext); 
		Z_SET_REFCOUNT_P(return_value, 1);
		Z_UNSET_ISREF_P(return_value); 
		display_object = (wingdi_devicecontext_object *) zend_objects_get_address(return_value TSRMLS_CC);
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

/** {{{ proto object \Win\Gdi\DeviceContext::selectObject(object obj)
        Selects obj into the device context.

		If object is a region:
			On success, returns SIMPLEREGION, COMPLEXREGION, or NULLREGION;
			On failure, returns HGDI_ERROR
		Other objects:
			On success, returns the previously selected object (or NULL, if there was no object)
			On failure, returns NULL.
*/
PHP_METHOD(WinGdiDeviceContext, selectObject)
{
	wingdi_devicecontext_object *dc_obj;
	zval *in_obj;

	WINGDI_ERROR_HANDLING();
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &in_obj) == FAILURE)
		return;
	WINGDI_RESTORE_ERRORS();

	dc_obj = (wingdi_devicecontext_object *) zend_object_store_get_object(getThis() TSRMLS_CC);

	if (Z_OBJCE_P(in_obj) == ce_wingdi_bitmap)
	{
		wingdi_bitmap_object *bitmap = zend_object_store_get_object(in_obj TSRMLS_CC);

		if (SelectObject(dc_obj->hdc, bitmap->bitmap_handle) == NULL)
		{
			php_error(E_ERROR, "SelectObject failed");
			return;
		}

		RETVAL_ZVAL(dc_obj->bitmap, 0, 0);
		dc_obj->bitmap = in_obj;
	}
	else if (Z_OBJCE_P(in_obj) == ce_wingdi_brush)
	{
		wingdi_brush_object *brush = zend_object_store_get_object(in_obj TSRMLS_CC);

		if (SelectObject(dc_obj->hdc, brush->brush_handle) == NULL)
		{
			php_error(E_ERROR, "SelectObject failed");
			return;
		}

		RETVAL_ZVAL(dc_obj->brush, 0, 0);
		dc_obj->brush = in_obj;
	}
	//else if (Z_OBJCE_P(in_obj) == ce_wingdi_font)
	//{
	//	wingdi_font_object *font = zend_object_store_get_object(in_obj TSRMLS_CC);

	//	if (SelectObject(dc_obj->hdc, font->font_handle) == NULL)
	//	{
	//		php_error(E_ERROR, "SelectObject failed");
	//		return;
	//	}

	//	RETVAL_ZVAL(dc_obj->font, 0, 0);
	//	dc_obj->font = in_obj;
	//}
	//else if (Z_OBJCE_P(in_obj) == ce_wingdi_pen)
	//{
	//	wingdi_pen_object *pen = zend_object_store_get_object(in_obj TSRMLS_CC);

	//	if (SelectObject(dc_obj->hdc, pen->pen_handle) == NULL)
	//	{
	//		php_error(E_ERROR, "SelectObject failed");
	//		return;
	//	}

	//	RETVAL_ZVAL(dc_obj->pen, 0, 0);
	//	dc_obj->pen = in_obj;
	//}
	else if (Z_OBJCE_P(in_obj) == ce_wingdi_region)
	{
		wingdi_region_object *region = zend_object_store_get_object(in_obj TSRMLS_CC);
		HGDIOBJ result               = SelectObject(dc_obj->hdc, region->region_handle);

		if (result == HGDI_ERROR)
		{
			php_error(E_ERROR, "SelectObject failed");
			return;
		}

		RETURN_LONG(HandleToLong(result));
	}
	else
	{
		php_error(E_ERROR, "got unexpected object %s", Z_OBJ_CLASS_NAME_P(in_obj));
	}
}

PHP_METHOD(WinGdiDeviceContext, getDisplayDevice)
{
    DISPLAY_DEVICE display_device_data;
    LPCTSTR device_name = NULL;
    DWORD device_num = 0,
          flags = 0;
    BOOL result;
    zval *z_device;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &z_device, &flags) != SUCCESS)
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

/** {{{ proto int Win\Gdi\DeviceContext::getBkMode()
        Returns the current background mix mode for the device context.
*/
PHP_METHOD(WinGdiDeviceContext, getBkMode)
{
	wingdi_devicecontext_object *dc = zend_object_store_get_object(getThis() TSRMLS_CC);

	WINGDI_ERROR_HANDLING();
	if (zend_parse_parameters_none() == FAILURE)
		return;
	WINGDI_RESTORE_ERRORS();

	RETURN_LONG(GetBkMode(dc->hdc));
}
/** }}} */

/** {{{ proto int Win\Gdi\DeviceContext::setBkMode(int mode)
		Sets the background mix mode for the device context.
*/
PHP_METHOD(WinGdiDeviceContext, setBkMode)
{
	wingdi_devicecontext_object *dc = zend_object_store_get_object(getThis() TSRMLS_CC);
	long mode;

	WINGDI_ERROR_HANDLING();
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &mode) == FAILURE)
		return;
	WINGDI_RESTORE_ERRORS();

	RETURN_LONG(SetBkMode(dc->hdc, mode));
}
/** }}} */

/** {{{ proto int Win\Gdi\DeviceContext::getROP2()
		Retrieves the foreground mix mode for the device context.
*/
PHP_METHOD(WinGdiDeviceContext, getROP2)
{
	wingdi_devicecontext_object *dc = zend_object_store_get_object(getThis() TSRMLS_CC);

	WINGDI_ERROR_HANDLING();
	if (zend_parse_parameters_none() == FAILURE)
		return;
	WINGDI_RESTORE_ERRORS();

	RETURN_LONG(GetROP2(dc->hdc));
}
/** }}} */

/** {{{ proto int Win\Gdi\DeviceContext::setROP2(int mode)
		Sets the foreground mix mode for the device context.
*/
PHP_METHOD(WinGdiDeviceContext, setROP2)
{
	wingdi_devicecontext_object *dc = zend_object_store_get_object(getThis() TSRMLS_CC);
	long mode;

	WINGDI_ERROR_HANDLING();
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &mode) == FAILURE)
		return;
	WINGDI_RESTORE_ERRORS();

	RETURN_LONG(SetROP2(dc->hdc, mode));
}
/** }}} */

/** {{{ proto int Win\Gdi\DeviceContext::getMapMode()
		Retrieves the mapping mode for the device context.
*/
PHP_METHOD(WinGdiDeviceContext, getMapMode)
{
	wingdi_devicecontext_object *dc = zend_object_store_get_object(getThis() TSRMLS_CC);

	WINGDI_ERROR_HANDLING();
	if (zend_parse_parameters_none() == FAILURE)
		return;
	WINGDI_RESTORE_ERRORS();

	RETURN_LONG(GetMapMode(dc->hdc));
}
/** }}} */

/** {{{ proto int Win\Gdi\DeviceContext::setMapMode(int mode)
		Sets the mapping mode for the device context.
*/
PHP_METHOD(WinGdiDeviceContext, setMapMode)
{
	wingdi_devicecontext_object *dc = zend_object_store_get_object(getThis() TSRMLS_CC);
	long mode;

	WINGDI_ERROR_HANDLING();
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &mode) == FAILURE)
		return;
	WINGDI_RESTORE_ERRORS();

	RETURN_LONG(SetMapMode(dc->hdc, mode));
}
/** }}} */

/** {{{ proto int Win\Gdi\DeviceContext::getPolyFillMode()
		Retrieves the polygon fill mode for the device context.
*/
PHP_METHOD(WinGdiDeviceContext, getPolyFillMode)
{
	wingdi_devicecontext_object *dc = zend_object_store_get_object(getThis() TSRMLS_CC);

	WINGDI_ERROR_HANDLING();
	if (zend_parse_parameters_none() == FAILURE)
		return;
	WINGDI_RESTORE_ERRORS();

	RETURN_LONG(GetPolyFillMode(dc->hdc));
}
/** }}} */

/** {{{ proto int Win\Gdi\DeviceContext::setPolyFillMode(int mode)
		Sets the polygon fill mode for the device context.
*/
PHP_METHOD(WinGdiDeviceContext, setPolyFillMode)
{
	wingdi_devicecontext_object *dc = zend_object_store_get_object(getThis() TSRMLS_CC);
	long mode;

	WINGDI_ERROR_HANDLING();
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &mode) == FAILURE)
		return;
	WINGDI_RESTORE_ERRORS();

	RETURN_LONG(SetPolyFillMode(dc->hdc, mode));
}
/** }}} */

/** {{{ proto int Win\Gdi\DeviceContext::getStretchBltMode()
		Retrieves the current stretching mode for the device context.
*/
PHP_METHOD(WinGdiDeviceContext, getStretchBltMode)
{
	wingdi_devicecontext_object *dc = zend_object_store_get_object(getThis() TSRMLS_CC);

	WINGDI_ERROR_HANDLING();
	if (zend_parse_parameters_none() == FAILURE)
		return;
	WINGDI_RESTORE_ERRORS();

	RETURN_LONG(GetStretchBltMode(dc->hdc));
}
/** }}} */

/** {{{ proto int Win\Gdi\DeviceContext::setStretchBltMode(int mode)
		Sets the current stretching mode for the device context.
*/
PHP_METHOD(WinGdiDeviceContext, setStretchBltMode)
{
	wingdi_devicecontext_object *dc = zend_object_store_get_object(getThis() TSRMLS_CC);
	long mode;
	int  status;

	WINGDI_ERROR_HANDLING();
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &mode) == FAILURE)
		return;
	WINGDI_RESTORE_ERRORS();

	status = SetStretchBltMode(dc->hdc, mode);
	if (status == 0 || status == ERROR_INVALID_PARAMETER)
	{
		wingdi_create_error(GetLastError(), ce_wingdi_exception TSRMLS_CC);
		return;
	}

	RETURN_LONG(status)
}
/** }}} */

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
	PHP_ME(WinGdiDeviceContext, selectObject, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(WinGdiDeviceContext, getDisplayDevice, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(WinGdiDeviceContext, changeDisplaySettings, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(WinGdiDeviceContext, getBkMode, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(WinGdiDeviceContext, setBkMode, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(WinGdiDeviceContext, getROP2, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(WinGdiDeviceContext, setROP2, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(WinGdiDeviceContext, getPolyFillMode, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(WinGdiDeviceContext, setPolyFillMode, NULL, ZEND_ACC_PUBLIC)
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

	object                = emalloc(sizeof(wingdi_devicecontext_object));
	object->std.ce        = ce;
	object->std.guards    = NULL;
	object->hdc           = NULL;
	object->window_handle = NULL;
    object->constructed   = 0;
	
	object->bitmap = NULL;
	object->brush  = NULL;
	object->font   = NULL;
	object->path   = NULL;
	object->pen    = NULL;
	object->region = NULL;

	object_properties_init(&object->std, ce);

	retval.handle   = zend_objects_store_put(object, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) wingdi_devicecontext_free_storage, NULL TSRMLS_CC);
	object->handle  = retval.handle;
	retval.handlers = &wingdi_devicecontext_object_handlers;
	return retval;
}
/* }}} */

static zend_function * get_constructor (zval * object TSRMLS_DC)
{
    if (Z_OBJCE_P(object) == ce_wingdi_devicecontext)
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
    zval                  *_this      = getThis(),
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
	ce_wingdi_devicecontext = zend_register_internal_class(&ce TSRMLS_CC);
	ce_wingdi_devicecontext->create_object = wingdi_devicecontext_object_new;

    memcpy(&wingdi_devicecontext_object_handlers, zend_get_std_object_handlers(),
        sizeof(wingdi_devicecontext_object_handlers));
    wingdi_devicecontext_object_handlers.get_constructor = get_constructor;

    ctor_wrapper_func.type                 = ZEND_INTERNAL_FUNCTION;
    ctor_wrapper_func.common.function_name = "internal_construction_wrapper";
    ctor_wrapper_func.common.scope         = ce_wingdi_devicecontext;
    ctor_wrapper_func.common.fn_flags      = ZEND_ACC_PROTECTED;
    ctor_wrapper_func.common.prototype     = NULL;
    ctor_wrapper_func.common.required_num_args = 0;
    ctor_wrapper_func.common.arg_info      = NULL;
    ctor_wrapper_func.internal_function.handler = construction_wrapper;
    ctor_wrapper_func.internal_function.module  = EG(current_module);

	zend_declare_class_constant_long(ce_wingdi_devicecontext, "OPAQUE", sizeof("OPAQUE") - 1, OPAQUE TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "TRANSPARENT", sizeof("TRANSPARENT") - 1, TRANSPARENT TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "R2_BLACK", sizeof("R2_BLACK") - 1, R2_BLACK TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "R2_COPYPEN", sizeof("R2_COPYPEN") - 1, R2_COPYPEN TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "R2_MASKNOTPEN", sizeof("R2_MASKNOTPEN") - 1, R2_MASKNOTPEN TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "R2_MASKPEN", sizeof("R2_MASKPEN") - 1, R2_MASKPEN TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "R2_MASKPENNOT", sizeof("R2_MASKPENNOT") - 1, R2_MASKPENNOT TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "R2_MERGEPENNOT", sizeof("R2_MERGEPENNOT") - 1, R2_MERGEPENNOT TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "R2_MERGENOTPEN", sizeof("R2_MERGENOTPEN") - 1, R2_MERGENOTPEN TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "R2_MERGEPEN", sizeof("R2_MERGEPEN") - 1, R2_MERGEPEN TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "R2_NOP", sizeof("R2_NOP") - 1, R2_NOP TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "R2_NOT", sizeof("R2_NOT") - 1, R2_NOT TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "R2_NOTCOPYPEN", sizeof("R2_NOTCOPYPEN") - 1, R2_NOTCOPYPEN TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "R2_NOTMASKPEN", sizeof("R2_NOTMASKPEN") - 1, R2_NOTMASKPEN TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "R2_NOTMERGEPEN", sizeof("R2_NOTMERGEPEN") - 1, R2_NOTMERGEPEN TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "R2_NOTXORPEN", sizeof("R2_NOTXORPEN") - 1, R2_NOTXORPEN TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "R2_WHITE", sizeof("R2_WHITE") - 1, R2_WHITE TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "R2_XORPEN", sizeof("R2_XORPEN") - 1, R2_XORPEN TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "MM_ANISOTROPIC", sizeof("MM_ANISOTROPIC") - 1, MM_ANISOTROPIC TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "MM_HIENGLISH", sizeof("MM_HIENGLISH") - 1, MM_HIENGLISH TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "MM_HIMETRIC", sizeof("MM_HIMETRIC") - 1, MM_HIMETRIC TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "MM_ISOTROPIC", sizeof("MM_ISOTROPIC") - 1, MM_ISOTROPIC TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "MM_LOENGLISH", sizeof("MM_LOENGLISH") - 1, MM_LOENGLISH TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "MM_LOMETRIC", sizeof("MM_LOMETRIC") - 1, MM_LOMETRIC TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "MM_TEXT", sizeof("MM_TEXT") - 1, MM_TEXT TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "MM_TWIPS", sizeof("MM_TWIPS") - 1, MM_TWIPS TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "ALTERNATE", sizeof("ALTERNATE") - 1, ALTERNATE TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "WINDING", sizeof("WINDING") - 1, WINDING TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "BLACKONWHITE", sizeof("BLACKONWHITE") - 1, BLACKONWHITE TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "COLORONCOLOR", sizeof("COLORONCOLOR") - 1, COLORONCOLOR TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "HALFTONE", sizeof("HALFTONE") - 1, HALFTONE TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "STRETCH_ANDSCANS", sizeof("STRETCH_ANDSCANS") - 1, STRETCH_ANDSCANS TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "STRETCH_DELETESCANS", sizeof("STRETCH_DELETESCANS") - 1, STRETCH_DELETESCANS TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "STRETCH_HALFTONE", sizeof("STRETCH_HALFTONE") - 1, STRETCH_HALFTONE TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "STRETCH_ORSCANS", sizeof("STRETCH_ORSCANS") - 1, STRETCH_ORSCANS TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "WHITEONBLACK", sizeof("WHITEONBLACK") - 1, WHITEONBLACK TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "DCX_WINDOW", sizeof("DCX_WINDOW") - 1, DCX_WINDOW TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "DCX_CACHE", sizeof("DCX_CACHE") - 1, DCX_CACHE TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "DCX_PARENTCLIP", sizeof("DCX_PARENTCLIP") - 1, DCX_PARENTCLIP TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "DCX_CLIPSIBLINGS", sizeof("DCX_CLIPSIBLINGS") - 1, DCX_CLIPSIBLINGS TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "DCX_CLIPCHILDREN", sizeof("DCX_CLIPCHILDREN") - 1, DCX_CLIPCHILDREN TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "DCX_NORESETATTRS", sizeof("DCX_NORESETATTRS") - 1, DCX_NORESETATTRS TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "DCX_LOCKWINDOWUPDATE", sizeof("DCX_LOCKWINDOWUPDATE") - 1, DCX_LOCKWINDOWUPDATE TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "DCX_EXCLUDERGN", sizeof("DCX_EXCLUDERGN") - 1, DCX_EXCLUDERGN TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "DCX_INTERSECTRGN", sizeof("DCX_INTERSECTRGN") - 1, DCX_INTERSECTRGN TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "DCX_INTERSECTUPDATE", sizeof("DCX_INTERSECTUPDATE") - 1, DCX_INTERSECTUPDATE TSRMLS_CC);
	zend_declare_class_constant_long(ce_wingdi_devicecontext, "DCX_VALIDATE", sizeof("DCX_VALIDATE") - 1, DCX_VALIDATE TSRMLS_CC);

	return SUCCESS;
}
/* }}} */