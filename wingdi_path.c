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

// These are the path functions listed here:
// http://msdn.microsoft.com/en-us/library/dd162782(v=VS.85).aspx
// The path stuff is a bit scattered (lines & curves, filled shapes,
// and other stuff), however, unless a cleaner implementation occurs
// to me, they'll all be under one class, which seems to make sense.

PHP_METHOD(WinGdiPath, __construct)
{
    wingdi_path_object *path_obj = zend_object_store_get_object(getThis() TSRMLS_CC);
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
    path_obj->device_context = dc_zval;
    Z_ADDREF_P(dc_zval);
}

PHP_METHOD(WinGdiPath, abort)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);
    RETURN_BOOL(AbortPath(dc_obj->hdc));
}

PHP_METHOD(WinGdiPath, closeFigure)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);
    RETURN_BOOL(CloseFigure(dc_obj->hdc));
}

PHP_METHOD(WinGdiPath, fillPath)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);
    RETURN_BOOL(FillPath(dc_obj->hdc));
}

PHP_METHOD(WinGdiPath, flattenPath)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);
    RETURN_BOOL(FlattenPath(dc_obj->hdc));
}

PHP_METHOD(WinGdiPath, getMiterLimit)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;
    FLOAT limit;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);
    RETURN_DOUBLE(GetMiterLimit(dc_obj->hdc, (PFLOAT)&limit));
}

PHP_METHOD(WinGdiPath, getPath)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;
    LPPOINT points;
    LPBYTE  types;
    zval *point_array;
    int result, i,
        points_total = 0;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj   = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);

    // Determine how many points exist in the path
    points_total = GetPath(dc_obj->hdc, NULL, NULL, 0);
    points = emalloc(sizeof(POINT) * points_total);
    types  = emalloc(sizeof(BYTE) * points_total);
    result = GetPath(dc_obj->hdc, points, types, points_total);

    array_init(return_value);
    for (i = 0; i < points_total; i++)
    {
        MAKE_STD_ZVAL(point_array);
        add_next_index_long(point_array, points[i].x);
        add_next_index_long(point_array, points[i].y);
        // Could the char -> long conversion here cause problems?
        add_next_index_long(point_array, types[i]);
        // Store this array in parent array
        add_next_index_zval(return_value, point_array);
    }
}

PHP_METHOD(WinGdiPath, toRegion)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_region_object *reg_obj;
    wingdi_path_object *path_obj;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);
    // Create a Region object - we can then grab this from the object store
    // using its handle.
    object_init_ex(return_value, ce_wingdi_region);
    reg_obj = zend_object_store_get_object_by_handle(Z_OBJ_HANDLE_P(return_value) TSRMLS_CC);

    reg_obj->region_handle = PathToRegion(dc_obj->hdc);
    if (!reg_obj->region_handle)
    {
        wingdi_create_error(GetLastError(), ce_wingdi_exception TSRMLS_CC);
        return;
    }
}

PHP_METHOD(WinGdiPath, setMiterLimit)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;
    FLOAT limit;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &limit) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);
    // The last param for SetMiterLimit is populated with the old limit.
    // Should we use it?
    RETURN_BOOL(SetMiterLimit(dc_obj->hdc, limit, NULL));
}

PHP_METHOD(WinGdiPath, strokeAndFill)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);
    RETURN_BOOL(StrokeAndFillPath(dc_obj->hdc));
}

PHP_METHOD(WinGdiPath, stroke)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);
    RETURN_BOOL(StrokePath(dc_obj->hdc));
}

PHP_METHOD(WinGdiPath, widen)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);
    RETURN_BOOL(WidenPath(dc_obj->hdc));
}

// Now for the filled-shape functions.
// I suppose these could be split into their own file (path_shapes)

static const zend_function_entry wingdi_path_functions[] = {
    PHP_ME(WinGdiPath, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(WinGdiPath, abort, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(WinGdiPath, closeFigure, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(WinGdiPath, fillPath, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(WinGdiPath, flattenPath, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(WinGdiPath, getMiterLimit, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(WinGdiPath, getPath, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(WinGdiPath, toRegion, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(WinGdiPath, setMiterLimit, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(WinGdiPath, strokeAndFill, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(WinGdiPath, stroke, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(WinGdiPath, widen, NULL, ZEND_ACC_PUBLIC)
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