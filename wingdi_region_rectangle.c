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

static zend_object_handlers object_handlers;
static zend_class_entry     *ce_wingdi_rect_region;
static zend_function        ctor_wrapper_func;

/* ----------------------------------------------------------------
  Win\Gdi\Region\Rectangle Userland API                                                    
------------------------------------------------------------------*/

ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_rect_region, 0, 0, 4)
    ZEND_ARG_INFO(0, left)
    ZEND_ARG_INFO(0, top)
    ZEND_ARG_INFO(0, right)
    ZEND_ARG_INFO(0, bottom)
ZEND_END_ARG_INFO()

/* {{{ proto Win\Gdi\Region\Rectangle Win\Gdi\Region\Rectangle->__construct(int x1, int y1, int x2, int y2)
       Creates a rectangular region
*/
PHP_METHOD(WinGdiRegionRect, __construct)
{
    wingdi_region_object *reg_obj;
    int left, top,
        right, bottom;

    WINGDI_ERROR_HANDLING()
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llll", &left, &top, &right, &bottom) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS()

    reg_obj = zend_object_store_get_object(getThis() TSRMLS_CC);
    reg_obj->region_handle = CreateRectRgn(left, top, right, bottom);
    reg_obj->constructed   = TRUE;
}
/* }}} */

static const zend_function_entry wingdi_region_rect_functions[] = {
    PHP_ME(WinGdiRegionRect, __construct, arginfo_wingdi_rect_region, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    {NULL, NULL, NULL}
};

/* ----------------------------------------------------------------
  Win\Gdi\Region\RectangleLifeCycle Functions                                                    
------------------------------------------------------------------*/

/* Override the default region constructor to change this region's handlers */
static zend_object_value wingdi_region_rectangle_new (zend_class_entry * ce TSRMLS_DC)
{
    zend_object_value object = wingdi_region_object_new(ce TSRMLS_CC);

    object.handlers = &object_handlers;

    return object;
}

static zend_function * get_constructor (zval * object TSRMLS_DC)
{
    if (Z_OBJCE_P(object) == ce_wingdi_rect_region)
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

PHP_MINIT_FUNCTION(wingdi_region_rectangle)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_REGION_NS, "Rectangle", wingdi_region_rect_functions);
    ce_wingdi_rect_region = 
        zend_register_internal_class_ex(&ce, ce_wingdi_region, PHP_WINGDI_REGION_NS TSRMLS_CC);
    ce_wingdi_rect_region->create_object = wingdi_region_rectangle_new;

    memcpy(&object_handlers,
        zend_get_std_object_handlers(), 
        sizeof(zend_object_handlers));
    object_handlers.get_constructor = get_constructor;

    ctor_wrapper_func.type                 = ZEND_INTERNAL_FUNCTION;
    ctor_wrapper_func.common.function_name = "internal_construction_wrapper";
    ctor_wrapper_func.common.scope         = ce_wingdi_rect_region;
    ctor_wrapper_func.common.fn_flags      = ZEND_ACC_PROTECTED;
    ctor_wrapper_func.common.prototype     = NULL;
    ctor_wrapper_func.common.required_num_args = 0;
    ctor_wrapper_func.common.arg_info      = NULL;
    ctor_wrapper_func.internal_function.handler = construction_wrapper;
    ctor_wrapper_func.internal_function.module  = EG(current_module);

    return SUCCESS;
}