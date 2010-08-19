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

zend_class_entry *ce_wingdi_elliptic_region;

/* ----------------------------------------------------------------
  Win\Gdi\Region\Elliptic Userland API                                                    
------------------------------------------------------------------*/

ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_elliptic_region, 0, 0, 4)
    ZEND_ARG_INFO(0, left)
    ZEND_ARG_INFO(0, top)
    ZEND_ARG_INFO(0, right)
    ZEND_ARG_INFO(0, bottom)
ZEND_END_ARG_INFO()

/* {{{ proto Win\Gdi\Region\Elliptic Win\Gdi\Region\Elliptic->__construct(int x1, int y1, int x2, int y2)
       Creates an elliptical region
*/
PHP_METHOD(WinGdiRegionElliptic, __construct)
{
    wingdi_region_object *reg_obj;
    int left, top,
        right, bottom;

    WINGDI_ERROR_HANDLING()
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llll", &left, &top, &right, &bottom) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS()

    reg_obj = (wingdi_region_object *)wingdi_region_object_get(getThis() TSRMLS_CC);
    reg_obj->region_handle = CreateEllipticRgn(left, top, right, bottom);
    if (reg_obj->region_handle == NULL)
    {
        wingdi_create_error(GetLastError(), ce_wingdi_exception TSRMLS_CC);
        return;
    }
}
/* }}} */

static const zend_function_entry wingdi_region_elliptic_functions[] = {
    PHP_ME(WinGdiRegionElliptic, __construct, arginfo_wingdi_elliptic_region, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    {NULL, NULL, NULL}
};

/* ----------------------------------------------------------------
  Win\Gdi\Region\Elliptic LifeCycle Functions                                                    
------------------------------------------------------------------*/

PHP_MINIT_FUNCTION(wingdi_region_elliptic)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_REGION_NS, "Elliptic", wingdi_region_elliptic_functions);
    ce_wingdi_elliptic_region = 
        zend_register_internal_class_ex(&ce, ce_wingdi_region, PHP_WINGDI_REGION_NS TSRMLS_CC);
    ce_wingdi_elliptic_region->create_object = wingdi_region_object_new;

    return SUCCESS;
}
