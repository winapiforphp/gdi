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
  | Author: Mark G. Skilbeck <markskilbeck@php.net                       |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#include "php_wingdi.h"
#include <zend_exceptions.h>

zend_class_entry *ce_wingdi_path;

/* ----------------------------------------------------------------
  Win\Gdi\Bitmap Userland API
------------------------------------------------------------------*/

PHP_METHOD(WinGdiPath, __construct)
{
    wingdi_path_object *path_obj;
    wingdi_devicecontext_object *dc_obj;
    zval *dc_zval;
    BOOL result;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &dc_zval) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    dc_obj = wingdi_devicecontext_object_get(dc_zval TSRMLS_CC);
    result = BeginPath(dc_obj->hdc);
    // Is this the correct way to handle this error?
    if (result == 0)
        zend_throw_exception(ce_wingdi_exception, "error creating path", 0 TSRMLS_CC);
}

static const zend_function_entry wingdi_path_functions[] = {
    PHP_ME(WinGdiPath, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    {NULL, NULL, NULL}
};

/* ----------------------------------------------------------------
  Win\Gdi\Path Custom Object magic
------------------------------------------------------------------*/

void wingdi_path_object_destroy(void * object TSRMLS_DC)
{
    wingdi_path_object *path = (wingdi_path_object *)object;

    zend_hash_destroy(path->std.properties);
    FREE_HASHTABLE(path->std.properties);

    efree(path);
}

zend_object_value wingdi_path_object_new(zend_class_entry *ce TSRMLS_DC)
{
    zend_object_value  ret;
    wingdi_path_object *path_obj;
    zval               *tmp;

    path_obj = emalloc(sizeof(wingdi_path_object));
    path_obj->std.ce = ce;
    path_obj->handle = zend_objects_store_put(
        path_obj, 
        (zend_objects_store_dtor_t)zend_objects_destroy_object,
        (zend_objects_free_object_storage_t)wingdi_path_object_destroy,
        NULL
        TSRMLS_CC
    );
    
    ret.handle = path_obj->handle;
    ret.handlers = zend_get_std_object_handlers();

    ALLOC_HASHTABLE(path_obj->std.properties);
    zend_hash_init(path_obj->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
    zend_hash_copy(path_obj->std.properties, &ce->default_properties,
        (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));

    return ret;
}

/* ----------------------------------------------------------------
  Win\Gdi\Path Lifecycle functions
------------------------------------------------------------------*/

PHP_MINIT_FUNCTION(wingdi_path)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_NS, "Path", wingdi_path_functions);
    ce_wingdi_path = zend_register_internal_class(&ce TSRMLS_CC);
    ce_wingdi_path->create_object;

    return SUCCESS;
}