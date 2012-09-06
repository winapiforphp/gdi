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

#ifndef PHP_WINGDI_API_H
#define PHP_WINGDI_API_H

#ifndef PHP_WINGDI_API
#define PHP_WINGDI_API __declspec(dllimport)
#endif

/* ----------------------------------------------------------------
  Namespace definitions
------------------------------------------------------------------*/
#define PHP_WINGDI_NS                ZEND_NS_NAME("Win", "Gdi")
#define PHP_WINGDI_BITMAP_NS         ZEND_NS_NAME(PHP_WINGDI_NS, "Bitmap")
#define PHP_WINGDI_BRUSH_NS          ZEND_NS_NAME(PHP_WINGDI_NS, "Brush")
#define PHP_WINGDI_COLOR_NS          ZEND_NS_NAME(PHP_WINGDI_NS, "Color")
#define PHP_WINGDI_DISPLAYCONTEXT_NS ZEND_NS_NAME(PHP_WINGDI_NS, "DeviceContext")
#define PHP_WINGDI_PEN_NS            ZEND_NS_NAME(PHP_WINGDI_NS, "Pen")
#define PHP_WINGDI_REGION_NS         ZEND_NS_NAME(PHP_WINGDI_NS, "Region")
#define PHP_WINGDI_REGION_POLY_NS    ZEND_NS_NAME(PHP_WINGDI_REGION_NS, "Polygon")

/* ----------------------------------------------------------------
  Typedefs
------------------------------------------------------------------*/
typedef struct _wingdi_window_object {
	zend_object  std;
	zend_bool    is_constructed;
	HashTable    *prop_handler;
	HWND         window_handle;
} wingdi_window_object;

typedef struct _wingdi_brush_object {
	zend_object	std;
	zend_bool	is_constructed;
	HBRUSH		brush_handle;
	BOOL		system_brush;
	HashTable	*prop_handler;
} wingdi_brush_object;

/* ----------------------------------------------------------------
  Exported Class Entries
------------------------------------------------------------------*/
extern PHP_WINGDI_API zend_class_entry *ce_wingdi_window;
extern PHP_WINGDI_API zend_class_entry *ce_wingdi_brush;

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */