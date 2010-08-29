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

zend_class_entry *ce_wingdi_round_region;

/* ----------------------------------------------------------------
  Win\Gdi\Region\RoundedRectangle Userland API                                                    
------------------------------------------------------------------*/

ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_rounded_rect_region, 0, 0, 6)
    ZEND_ARG_INFO(0, left)
    ZEND_ARG_INFO(0, top)
    ZEND_ARG_INFO(0, right)
    ZEND_ARG_INFO(0, bottom)
    ZEND_ARG_INFO(0, width)
    ZEND_ARG_INFO(0, height)
ZEND_END_ARG_INFO()

/* {{{ proto Win\Gdi\Region\RoundedRectangle Win\Gdi\Region\RoundedRectangle->__construct(int x1, int y1, int x2, int y2, int width, int height)
       Creates a rectangular region with rounded corners
*/
PHP_METHOD(WinGdiRegionRoundedRect, __construct)
{
    wingdi_region_object *reg_obj;
    int left, top,
        right, bottom,
        width, height;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llllll", 
            &left, &top, &right, &bottom, &width, &height) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    reg_obj = (wingdi_region_object *)zend_object_store_get_object(getThis() TSRMLS_CC);
    reg_obj->region_handle = CreateRoundRectRgn(left, top, right, bottom, width, height);
    if (reg_obj->region_handle == NULL)
    {
        wingdi_create_error(GetLastError(), ce_wingdi_exception TSRMLS_CC);
        return;
    }
}
/* }}} */

static const zend_function_entry wingdi_region_rounded_rect_functions[] = {
    PHP_ME(WinGdiRegionRoundedRect, __construct, arginfo_wingdi_rounded_rect_region, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    {NULL, NULL, NULL}
};

/* ----------------------------------------------------------------
  Win\Gdi\Region\RoundedRectangle LifeCycle Functions                                                    
------------------------------------------------------------------*/

PHP_MINIT_FUNCTION(wingdi_region_roundedrectangle)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_REGION_NS, "RoundedRectangle", wingdi_region_rounded_rect_functions);
    ce_wingdi_round_region =
        zend_register_internal_class_ex(&ce, ce_wingdi_region, PHP_WINGDI_REGION_NS TSRMLS_CC);
    ce_wingdi_round_region->create_object = wingdi_region_object_new;

    return SUCCESS;
}