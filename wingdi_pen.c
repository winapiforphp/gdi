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
extern zend_class_entry *ce_WinGdiColor;

/* Custom Object junk */
zend_class_entry *ce_WinGdiPen;
zend_object_handlers wingdipen_object_handlers;
void wingdipen_write_property(zval *object, zval *member, zval *value TSRMLS_DC);
zval *wingdipen_read_property(zval *object, zval *member, int type TSRMLS_DC);

/* Methods for the class */
PHP_METHOD(WinGdiPen, __construct);

/* arginfo for the class */
ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdipen___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, style)
	ZEND_ARG_INFO(0, width)
	ZEND_ARG_OBJ_INFO(0, color, Win::Gdi::Color, 0)
	ZEND_ARG_OBJ_INFO(0, brush, Win::Gdi::Brush, 0)
ZEND_END_ARG_INFO();

/* regular Window methods */
static const zend_function_entry wingdipen_functions[] = {
	PHP_ME(WinGdiPen, __construct, arginfo_wingdipen___construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	{NULL, NULL, NULL}
};

/* {{{ __construct */
PHP_METHOD(WinGdiPen, __construct)
{
	int style = 0, width = 1;
	zval *color, *brush;
	zend_object *pen_obj = (zend_object*)zend_object_store_get_object(getThis() TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llO|O", &style, &width, &color, ce_WinGdiColor, &brush) == FAILURE) {
		return;
	}

	/* Sanity check width */
	if (width < 0)
	{
		width = 1;
	}

	php_printf("My width is %d\n", width);
	php_printf("My style is %d\n", style);
	php_printf("My color is something \n");

	return;
}
/* }}} */

PHP_MINIT_FUNCTION(wingdi_pen)
{
	zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_NS, "Pen", wingdipen_functions);
	ce_WinGdiPen = zend_register_internal_class(&ce TSRMLS_CC);

	memcpy(&wingdipen_object_handlers, &zend_get_std_object_handlers, sizeof(zend_object_handlers));

	/* Override additional handlers (read_prop/write_prop/etc...) here */
	//wingdipen_object_handlers.read_property  = wingdipen_read_property;
	//wingdipen_object_handlers.write_property = wingdipen_write_property;

	/* Constants -  */
	zend_declare_class_constant_long(ce_WinGdiPen, "GEOMETRIC", sizeof("GEOMETRIC") - 1, PS_GEOMETRIC TSRMLS_CC);
	zend_declare_class_constant_long(ce_WinGdiPen, "COSMETIC", sizeof("COSMETIC") - 1, PS_COSMETIC TSRMLS_CC);
	zend_declare_class_constant_long(ce_WinGdiPen, "ALTERNATE", sizeof("ALTERNATE") - 1, PS_ALTERNATE TSRMLS_CC);
	zend_declare_class_constant_long(ce_WinGdiPen, "SOLID", sizeof("SOLID") - 1, PS_SOLID TSRMLS_CC);
	zend_declare_class_constant_long(ce_WinGdiPen, "DASH", sizeof("DASH") - 1, PS_DASH TSRMLS_CC);
	zend_declare_class_constant_long(ce_WinGdiPen, "DOT", sizeof("DOT") - 1, PS_DOT TSRMLS_CC);
	zend_declare_class_constant_long(ce_WinGdiPen, "DASHDOT", sizeof("DASHDOT") - 1, PS_DASHDOT TSRMLS_CC);
	zend_declare_class_constant_long(ce_WinGdiPen, "DASHDOTDOT", sizeof("DASHDOTDOT") - 1, PS_DASHDOTDOT TSRMLS_CC);
	zend_declare_class_constant_long(ce_WinGdiPen, "NULL", sizeof("NULL") - 1, PS_NULL TSRMLS_CC);
	zend_declare_class_constant_long(ce_WinGdiPen, "USERSTYLE", sizeof("USERSTYLE") - 1, PS_USERSTYLE TSRMLS_CC);
	zend_declare_class_constant_long(ce_WinGdiPen, "INSIDEFRAME", sizeof("INSIDEFRAME") - 1, PS_INSIDEFRAME TSRMLS_CC);
	zend_declare_class_constant_long(ce_WinGdiPen, "ENDCAP_ROUND", sizeof("ENDCAP_ROUND") - 1, PS_ENDCAP_ROUND TSRMLS_CC);
	zend_declare_class_constant_long(ce_WinGdiPen, "ENDCAP_SQUARE", sizeof("ENDCAP_SQUARE") - 1, PS_ENDCAP_SQUARE TSRMLS_CC);
	zend_declare_class_constant_long(ce_WinGdiPen, "ENDCAP_FLAT", sizeof("ENDCAP_FLAT") - 1, PS_ENDCAP_FLAT TSRMLS_CC);
	zend_declare_class_constant_long(ce_WinGdiPen, "JOIN_BEVEL", sizeof("JOIN_BEVEL") - 1, PS_JOIN_BEVEL TSRMLS_CC);
	zend_declare_class_constant_long(ce_WinGdiPen, "JOIN_MITER", sizeof("JOIN_MITER") - 1, PS_JOIN_MITER TSRMLS_CC);
	zend_declare_class_constant_long(ce_WinGdiPen, "JOIN_ROUND", sizeof("JOIN_ROUND") - 1, PS_JOIN_ROUND TSRMLS_CC);

	return SUCCESS;
}