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
#include "zend_exceptions.h"

zend_class_entry *ce_WinGdiColor;
zend_object_handlers wingdi_color_object_handlers;


/* ----------------------------------------------------------------
  \Win\Gdi\Color Userland API                                                      
------------------------------------------------------------------*/

/* {{{ proto void \Win\Gdi\Color->__construct(int red, int green, int blue)
	   void \Win\Gdi\Color->__construct(string hexcolor)
	   void \Win\Gdi\Color->__construct(int hexcolor)

       Manages a color in RGB format or hex string or integer
 */
PHP_METHOD(WinGdiColor, __construct)
{
	zval *colval = NULL;
	long red=0, green=0, blue=0;
	long hex;
	
	zend_object *color_obj = (zend_object*)zend_object_store_get_object(getThis() TSRMLS_CC);

    WINGDI_ERROR_HANDLING()
	/* we can have no args which creates a black object
	   or have one arg which can be a string (with or without #) or long color value
	   or 1, 2 or 3 args as long values */
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zll", &colval, &green, &blue) == FAILURE)
		return;
	
	if(colval) {
		switch(Z_TYPE_P(colval)) {
			case IS_STRING:
				if(Z_STRLEN_P(colval) == 7) {
					char hash;
					sscanf_s(Z_STRVAL_P(colval),"%1c%2x%2x%2x",  &hash, sizeof(char), &red, &green, &blue);
				} else if(Z_STRLEN_P(colval) == 6) {
					sscanf_s(Z_STRVAL_P(colval),"%2x%2x%2x", &red, &green, &blue);
				} else if(Z_STRLEN_P(colval) == 4) {
					char hash;
					sscanf_s(Z_STRVAL_P(colval),"%1c%1x%1x%1x", &hash, sizeof(char), &red, &green, &blue);
					red <<= 8;
					green <<= 8;
					blue <<= 8;
				} else if (Z_STRLEN_P(colval) == 3) {
					sscanf_s(Z_STRVAL_P(colval), "%1x%1x%1x", &red, &green, &blue);
					red <<= 8;
					green <<= 8;
					blue <<= 8;
				} else {
					zend_throw_exception(ce_wingdi_argexception, "Invalid color string", 0 TSRMLS_CC);
				}
			break;
			case IS_LONG:
				if(ZEND_NUM_ARGS() > 1) {
					red = Z_LVAL_P(colval);
				} else {
					red = GetRValue(Z_LVAL_P(colval));
					green = GetGValue(Z_LVAL_P(colval));
					blue = GetBValue(Z_LVAL_P(colval));
				}
			break;
			default:
				zend_throw_exception(ce_wingdi_argexception, "Invalid color, must be string or integer", 0 TSRMLS_CC);

		}
	}
    WINGDI_RESTORE_ERRORS()

	/* rgb sanity checking */
	red = MAX(0,MIN(255,red));
	green = MAX(0,MIN(255,green));
	blue = MAX(0,MIN(255,blue));
	hex = RGB(red,  green,  blue);

	/* store in properties */
	zend_update_property_long(ce_WinGdiColor, getThis(), "red", sizeof("red") - 1, red TSRMLS_CC);
	zend_update_property_long(ce_WinGdiColor, getThis(), "green", sizeof("green") - 1, green TSRMLS_CC);
	zend_update_property_long(ce_WinGdiColor, getThis(), "blue", sizeof("blue") - 1, blue TSRMLS_CC);
	zend_update_property_long(ce_WinGdiColor, getThis(), "hex", sizeof("hex") - 1, hex TSRMLS_CC);

	return;
}
/* }}} */

/* {{{ __toString */
PHP_METHOD(WinGdiColor, __toString)
{
	zval *hex;
	char *str;

	MAKE_STD_ZVAL(hex);
	str = emalloc(7);

	hex = zend_read_property(ce_WinGdiColor, getThis(), "hex", sizeof("hex")-1, 1 TSRMLS_CC);
	sprintf(str, "#%02x%02x%02x", GetRValue(Z_LVAL_P(hex)), GetBValue(Z_LVAL_P(hex)), GetGValue(Z_LVAL_P(hex)));
	zval_dtor(hex);
	RETURN_STRINGL(str, 7, 0);
}
/* }}} */

/* {{{ Method definitions */
ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_color___construct, 0, 0, 0)
	ZEND_ARG_INFO(0, red)
	ZEND_ARG_INFO(0, green)
	ZEND_ARG_INFO(0, blue)
ZEND_END_ARG_INFO();

static const zend_function_entry wingdi_color_functions[] = {
	PHP_ME(WinGdiColor, __construct, arginfo_wingdi_color___construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(WinGdiColor, __toString, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};
/* }}} */

/* ----------------------------------------------------------------
  \Win\Gdi\DisplayContext Custom Object handlers                                                 
------------------------------------------------------------------*/

/* {{{ wingdi_color_object_new
       only needed so handlers are assigned properly */
zend_object_value wingdi_color_object_new(zend_class_entry *ce TSRMLS_DC)
{
	zend_object *object;
	zend_object_value retval;
	zval tmp;

	retval = zend_objects_new(&object, ce TSRMLS_CC);
	ALLOC_HASHTABLE(object->properties);
	zend_hash_init(object->properties, 0, NULL, ZVAL_PTR_DTOR, 0);
	zend_hash_copy(object->properties, &ce->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));

	/* Initialize custom internal properties here */
	retval.handlers = &wingdi_color_object_handlers;

	return retval;
}
/* }}} */

/* {{{ wingdi_color_write_property
      intercept red/green/blue/hex and ignore everything else */
static void wingdi_color_write_property(zval *object, zval *member, zval *value TSRMLS_DC)
{
	long red=0, green=0, blue=0;
	zval *r, *g, *b, *rname, *gname, *bname, *hex, *hname;

	convert_to_string(member);
	
	if((strcmp(Z_STRVAL_P(member), "red") == 0) || (strcmp(Z_STRVAL_P(member), "green") == 0) ||
		(strcmp(Z_STRVAL_P(member), "blue") == 0)) {
		
		convert_to_long(value);
		// sanity checking
		Z_LVAL_P(value) = MAX(0,MIN(255,Z_LVAL_P(value)));
		std_object_handlers.write_property(object, member, value TSRMLS_CC);

		// grab red/green/blue values
		MAKE_STD_ZVAL(rname);
		MAKE_STD_ZVAL(gname);
		MAKE_STD_ZVAL(bname);
		ZVAL_STRING(rname, "red", 0);
		ZVAL_STRING(gname, "green", 0);
		ZVAL_STRING(bname, "blue", 0);
		r = std_object_handlers.read_property(object, rname, IS_LONG TSRMLS_CC);
		g = std_object_handlers.read_property(object, gname, IS_LONG TSRMLS_CC);
		b = std_object_handlers.read_property(object, bname, IS_LONG TSRMLS_CC);

		// create the new hex value and write it
		MAKE_STD_ZVAL(hex);
		ZVAL_LONG(hex, RGB(Z_LVAL_P(r), Z_LVAL_P(g), Z_LVAL_P(b)));
		MAKE_STD_ZVAL(hname);
		ZVAL_STRING(hname, "hex", 0);
		std_object_handlers.write_property(object, hname, hex TSRMLS_CC);

	} else if(strcmp(Z_STRVAL_P(member), "hex") == 0) {
		if(Z_TYPE_P(value) == IS_STRING) {
			if(Z_STRLEN_P(value) == 7) {
				char hash;
				sscanf_s(Z_STRVAL_P(value),"%1c%2x%2x%2x",  &hash, sizeof(char), &red, &green, &blue);
			} else if(Z_STRLEN_P(value) == 6) {
				sscanf_s(Z_STRVAL_P(value),"%2x%2x%2x", &red, &green, &blue);
			} else if(Z_STRLEN_P(value) == 4) {
				char hash;
				sscanf_s(Z_STRVAL_P(value),"%1c%1x%1x%1x", &hash, sizeof(char), &red, &green, &blue);
				red <<= 8;
				green <<= 8;
				blue <<= 8;
			} else if (Z_STRLEN_P(value) == 3) {
				sscanf_s(Z_STRVAL_P(value), "%1x%1x%1x", &red, &green, &blue);
				red <<= 8;
				green <<= 8;
				blue <<= 8;
			} else {
				convert_to_long(value);
				red = GetRValue(Z_LVAL_P(value));
				green = GetGValue(Z_LVAL_P(value));
				blue = GetBValue(Z_LVAL_P(value));
			}
		} else {
			convert_to_long(value);
			red = GetRValue(Z_LVAL_P(value));
			green = GetGValue(Z_LVAL_P(value));
			blue = GetBValue(Z_LVAL_P(value));
		}

		// rgb sanity checking
		red = MAX(0,MIN(255,red));
		green = MAX(0,MIN(255,green));
		blue = MAX(0,MIN(255,blue));

		// rgb and members to zvals
		MAKE_STD_ZVAL(r);
		MAKE_STD_ZVAL(g);
		MAKE_STD_ZVAL(b);
		MAKE_STD_ZVAL(rname);
		MAKE_STD_ZVAL(gname);
		MAKE_STD_ZVAL(bname);
		ZVAL_LONG(r, red);
		ZVAL_LONG(g, green);
		ZVAL_LONG(b, blue);
		ZVAL_STRING(rname, "red", 0);
		ZVAL_STRING(gname, "green", 0);
		ZVAL_STRING(bname, "blue", 0);

		// store in properties
		std_object_handlers.write_property(object, rname, r TSRMLS_CC);
		std_object_handlers.write_property(object, bname, b TSRMLS_CC);
		std_object_handlers.write_property(object, gname, g TSRMLS_CC);

		// write hex
		ZVAL_LONG(value, RGB(red, green, blue));
		std_object_handlers.write_property(object, member, value TSRMLS_CC);
	} else {
		std_object_handlers.write_property(object, member, value TSRMLS_CC);
	}
}
/* }}} */

/* {{{ wingdi_color_delete_property
       disallows unsetting the special built in stuff */
static void wingdi_color_delete_property(zval *object, zval *member TSRMLS_DC)
{
	convert_to_string(member);

	if((strcmp(Z_STRVAL_P(member), "red") == 0) || (strcmp(Z_STRVAL_P(member), "green") == 0) ||
		(strcmp(Z_STRVAL_P(member), "blue") == 0) || (strcmp(Z_STRVAL_P(member), "hex") == 0))
	{
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Property %s cannot be unset", Z_STRVAL_P(member));
	}
	else
	{
		std_object_handlers.unset_property(object, member TSRMLS_CC);
	}
}
/* }}} */

/* ----------------------------------------------------------------
  \Win\Gdi\Color LifeCycle Functions                                                    
------------------------------------------------------------------*/

/* {{{ PHP_MINIT_FUNCTION(wingdi_color)
	Registers the \Win\Gdi\Color class
 */
PHP_MINIT_FUNCTION(wingdi_color)
{
	zend_class_entry ce;

	memcpy(&wingdi_color_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	wingdi_color_object_handlers.write_property = wingdi_color_write_property;
	wingdi_color_object_handlers.unset_property = wingdi_color_delete_property;

    INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_NS, "Color", wingdi_color_functions);
	ce_WinGdiColor = zend_register_internal_class(&ce TSRMLS_CC);
	ce_WinGdiColor->create_object  = wingdi_color_object_new;

	zend_declare_property_long(ce_WinGdiColor, "red", sizeof("red") - 1, 0, ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_long(ce_WinGdiColor, "green", sizeof("green") - 1, 0, ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_long(ce_WinGdiColor, "blue", sizeof("blue") - 1, 0, ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_long(ce_WinGdiColor, "hex", sizeof("hex") - 1, 0, ZEND_ACC_PUBLIC TSRMLS_CC);

	return SUCCESS;
}
/* }}} */