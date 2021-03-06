/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2012 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Elizabeth Smith <auroraeosrose@php.net>                      |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_WINGDI_H
#define PHP_WINGDI_H

/* version info file */
#include "version.h"

/* Needed PHP includes */
#include "php.h"

#ifdef PHP_WIN32
#define PHP_WINGDI_API __declspec(dllexport)
#else
#define PHP_WINGDI_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#define PHP_WINGDI_NS                ZEND_NS_NAME("Win", "Gdi")
#define PHP_WINGDI_BITMAP_NS         ZEND_NS_NAME(PHP_WINGDI_NS, "Bitmap")
#define PHP_WINGDI_BRUSH_NS          ZEND_NS_NAME(PHP_WINGDI_NS, "Brush")
#define PHP_WINGDI_COLOR_NS          ZEND_NS_NAME(PHP_WINGDI_NS, "Color")
#define PHP_WINGDI_DISPLAYCONTEXT_NS ZEND_NS_NAME(PHP_WINGDI_NS, "DeviceContext")
#define PHP_WINGDI_PEN_NS            ZEND_NS_NAME(PHP_WINGDI_NS, "Pen")
#define PHP_WINGDI_REGION_NS         ZEND_NS_NAME(PHP_WINGDI_NS, "Region")
#define PHP_WINGDI_REGION_POLY_NS    ZEND_NS_NAME(PHP_WINGDI_REGION_NS, "Polygon")

/** Shortens the amount of typing needed, a little */
#define WINGDI_ERROR_HANDLING() \
    zend_error_handling error_handling; \
    zend_replace_error_handling(EH_THROW, ce_wingdi_argexception, &error_handling TSRMLS_CC); 

#define WINGDI_RESTORE_ERRORS() \
    zend_restore_error_handling(&error_handling TSRMLS_CC);

/* ----------------------------------------------------------------
  Typedefs                                               
------------------------------------------------------------------*/

/* Class structs */
typedef struct _wingdi_window_object {
	zend_object  std;
	zend_bool    is_constructed;
	HashTable    *prop_handler;
	HWND         window_handle;
} wingdi_window_object;

typedef struct _wingdi_devicecontext_object {
	zend_object  std;
    zend_bool    constructed;
	HDC          hdc;
	HWND         window_handle;
	//HashTable *prop_handler;
	zend_object_handle handle;

	// The DC's graphics objects are cached here. DC::selectObject() should return
	// the previously selected object, a la SelectObject().
	zval *bitmap;
	zval *brush;
	zval *font;
	zval *path;
	zval *pen;
	zval *region;
} wingdi_devicecontext_object;

typedef struct _wingdi_bitmap_object {
	zend_object  std;
	HGDIOBJ bitmap_handle;
	zend_object_handle handle;
} wingdi_bitmap_object;

typedef struct _wingdi_path_object {
    zend_object std;
    zend_bool   constructed;
    // The path stuff all requires a device context object, so, instead
    // of having the DC passed in via a parameter to all the path methods
    // (ick), have the user provide a DC with the path constructor, and 
    // cache that DC here and use it when needed.
    zval *device_context;
    zend_object_handle handle;
} wingdi_path_object;

typedef struct _wingdi_region_object {
	zend_object std;
    zend_bool   constructed;
	HRGN		region_handle;
	zend_object_handle handle;
} wingdi_region_object;

typedef struct _wingdi_brush_object {
	zend_object	std;
	zend_bool	is_constructed;
	HBRUSH		brush_handle;
	BOOL		system_brush;
	HashTable	*prop_handler;
} wingdi_brush_object;

/* ----------------------------------------------------------------
  C API                                             
------------------------------------------------------------------*/
void wingdi_create_error(int error, zend_class_entry *ce TSRMLS_DC);
int  wingdi_juggle_type(zval *value, int type TSRMLS_DC);
zend_object_value wingdi_region_object_new(zend_class_entry *ce TSRMLS_DC);

/* ----------------------------------------------------------------
  Class Entries                                              
------------------------------------------------------------------*/
extern zend_class_entry *ce_wingdi_exception;
extern zend_class_entry *ce_wingdi_argexception;
extern zend_class_entry *ce_wingdi_versionexception;
extern zend_class_entry *ce_wingdi_bitmap;
extern zend_class_entry *ce_wingdi_region;
extern zend_class_entry *ce_wingdi_rect_region;
extern zend_class_entry *ce_wingdi_devicecontext;

extern PHP_WINGDI_API zend_class_entry *ce_wingdi_brush;
extern PHP_WINGDI_API zend_class_entry *ce_wingdi_window;

/* ----------------------------------------------------------------
  Object Globals, lifecycle and static linking                                                
------------------------------------------------------------------*/

/* Lifecycle Function Declarations */
PHP_MINIT_FUNCTION(wingdi_util);
PHP_MINIT_FUNCTION(wingdi_window);
PHP_MINIT_FUNCTION(wingdi_bitmap);
PHP_MINIT_FUNCTION(wingdi_brush);
PHP_MINIT_FUNCTION(wingdi_color);
PHP_MINIT_FUNCTION(wingdi_devicecontext);
PHP_MINIT_FUNCTION(wingdi_pen);
PHP_MINIT_FUNCTION(wingdi_path);
PHP_MINIT_FUNCTION(wingdi_region);
PHP_MINIT_FUNCTION(wingdi_region_rectangle);
PHP_MINIT_FUNCTION(wingdi_region_roundedrectangle);
PHP_MINIT_FUNCTION(wingdi_region_elliptic);
PHP_MINIT_FUNCTION(wingdi_region_polygon);

/* Required for static linking */
extern zend_module_entry wingdi_module_entry;
#define phpext_wingdi_ptr &wingdi_module_entry

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */