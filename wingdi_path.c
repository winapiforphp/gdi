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
// I suppose these could be split into their own file (path_shapes, path_lines_and_curves?)

PHP_METHOD(WinGdiPath, chord)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;
    int x1, y1, x2, y2,
        xr1, yr1, xr2, yr2;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llllllll", 
            &x1, &y1, &x2, &y2, &xr1, &yr1, &xr2, &yr2) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = zend_object_store_get_object(getThis() TSRMLS_CC);
    dc_obj   = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);

    RETURN_BOOL(Chord(dc_obj->hdc, x1, y1, x2, y2, xr1, yr1, xr2, yr2));
}

PHP_METHOD(WinGdiPath, ellipse)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;
    int x1, y1, x2, y2;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llll", &x1, &y1, &x2, &y2) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = zend_object_store_get_object(getThis() TSRMLS_CC);
    dc_obj   = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);

    RETURN_BOOL(Ellipse(dc_obj->hdc, x1, y1, x2, y2));
}

PHP_METHOD(WinGdiPath, fillRectangle)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_brush_object *br_obj;
    wingdi_path_object *path_obj;
    zval *coords_zval,
         *br_zval,
         **tmp;
    RECT rect_coords;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &br_zval, &coords_zval) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = zend_object_store_get_object(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);

    if (Z_TYPE_P(br_zval) == IS_OBJECT)
        br_obj = wingdi_brush_object_get(br_zval TSRMLS_CC);
    else 
    {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "expected an object for parameter 2, got %s",
            zend_zval_type_name(br_zval));
        return;
    }

    // Is this the best way to do this?
    zend_hash_index_find(Z_ARRVAL_P(coords_zval), 0, (void **)&tmp);
    rect_coords.top = Z_LVAL_PP(tmp);
    zend_hash_index_find(Z_ARRVAL_P(coords_zval), 1, (void **)&tmp);
    rect_coords.left = Z_LVAL_PP(tmp);
    zend_hash_index_find(Z_ARRVAL_P(coords_zval), 2, (void **)&tmp);
    rect_coords.bottom = Z_LVAL_PP(tmp);
    zend_hash_index_find(Z_ARRVAL_P(coords_zval), 3, (void **)&tmp);
    rect_coords.right = Z_LVAL_PP(tmp);   

    RETURN_BOOL(FillRect(dc_obj->hdc, &rect_coords, br_obj->brush_handle));
}

PHP_METHOD(WinGdiPath, frameRectangle)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_brush_object *br_obj;
    wingdi_path_object *path_obj;
    zval *coords_zval,
         *br_zval,
         **tmp;
    RECT rect_coords;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &br_zval, &coords_zval) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = zend_object_store_get_object(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);

    if (Z_TYPE_P(br_zval) == IS_OBJECT)
        br_obj = wingdi_brush_object_get(br_zval TSRMLS_CC);
    else 
    {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "expected an object for parameter 2, got %s",
            zend_zval_type_name(br_zval));
        return;
    }

    // Is this the best way to do this?
    zend_hash_index_find(Z_ARRVAL_P(coords_zval), 0, (void **)&tmp);
    rect_coords.top = Z_LVAL_PP(tmp);
    zend_hash_index_find(Z_ARRVAL_P(coords_zval), 1, (void **)&tmp);
    rect_coords.left = Z_LVAL_PP(tmp);
    zend_hash_index_find(Z_ARRVAL_P(coords_zval), 2, (void **)&tmp);
    rect_coords.bottom = Z_LVAL_PP(tmp);
    zend_hash_index_find(Z_ARRVAL_P(coords_zval), 3, (void **)&tmp);
    rect_coords.right = Z_LVAL_PP(tmp);   

    RETURN_BOOL(FrameRect(dc_obj->hdc, &rect_coords, br_obj->brush_handle));
}

PHP_METHOD(WinGdiPath, invertRectangle)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;
    zval *coords_zval,
         **tmp;
    RECT rect_coords;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &coords_zval) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = zend_object_store_get_object(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);

    // Is this the best way to do this?
    zend_hash_index_find(Z_ARRVAL_P(coords_zval), 0, (void **)&tmp);
    rect_coords.top = Z_LVAL_PP(tmp);
    zend_hash_index_find(Z_ARRVAL_P(coords_zval), 1, (void **)&tmp);
    rect_coords.left = Z_LVAL_PP(tmp);
    zend_hash_index_find(Z_ARRVAL_P(coords_zval), 2, (void **)&tmp);
    rect_coords.bottom = Z_LVAL_PP(tmp);
    zend_hash_index_find(Z_ARRVAL_P(coords_zval), 3, (void **)&tmp);
    rect_coords.right = Z_LVAL_PP(tmp);   

    RETURN_BOOL(InvertRect(dc_obj->hdc, &rect_coords));
}

PHP_METHOD(WinGdiPath, pie)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;
    int x1, y1, x2, y2,
        xr1, yr1, xr2, yr2;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llllllll", 
            &x1, &y1, &x2, &y2, &xr1, &yr1, &xr2, &yr2) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = zend_object_store_get_object(getThis() TSRMLS_CC);
    dc_obj   = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);

    RETURN_BOOL(Pie(dc_obj->hdc, x1, y1, x2, y2, xr1, yr1, xr2, yr2));
}

// A carbon copy of the Win\Gdi\Region\Polygon implementation
// without some parameter checks (no poly-fill-mode given)
PHP_METHOD(WinGdiRegionPolygon, polygon)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;
    HashPointer p;              // We use this to determine what index we are at in the array
    POINT *points = NULL;
    zval  ***parameters,
          **current_elem,
          **x, **y;
    INT *point_counts   = NULL; // Number of POINTs in corresponding points array
    int  ints_in_count  = 0,    // Number of ints in point_counts
         param_count,
         i, t = 0;              // Total number of POINTs

    WINGDI_ERROR_HANDLING();
    // Will throw an exception if no parameters are given
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &parameters, &param_count) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    point_counts = emalloc(param_count * sizeof(INT));

    for (i = 0; i < param_count; i++)
    {
        if (Z_TYPE_PP(parameters[i]) == IS_ARRAY)
        {
            point_counts[ints_in_count] = 0;

            for (zend_hash_internal_pointer_reset(Z_ARRVAL_PP(parameters[i]));
                 zend_hash_has_more_elements(Z_ARRVAL_PP(parameters[i])) == SUCCESS;
                 zend_hash_move_forward(Z_ARRVAL_PP(parameters[i])))
            {
                points = erealloc(points, t * sizeof(POINT) + sizeof(POINT));
                zend_hash_get_pointer(Z_ARRVAL_PP(parameters[i]), &p);
                zend_hash_get_current_data(Z_ARRVAL_PP(parameters[i]), (void **)&current_elem);

                if (Z_TYPE_PP(current_elem) != IS_ARRAY)
                {
                    php_error_docref(NULL TSRMLS_CC, E_ERROR,
                        "expected array in array for parameter %d, got %s", i + 1, zend_zval_type_name(*current_elem));
                    goto CLEANUP;
                }
                else
                {
                    if (zend_hash_num_elements(Z_ARRVAL_PP(current_elem)) != 2)
                    {
                        php_error_docref(NULL TSRMLS_CC, E_ERROR,
                            "expected point-array at index %d for parameter %d to have 2 elements, %d given",
                            p.h, i + 1, zend_hash_num_elements(Z_ARRVAL_PP(current_elem)));
                        goto CLEANUP;
                    }
                    else
                    {
                        zend_hash_index_find(Z_ARRVAL_PP(current_elem), 0, (void **)&x);
                        zend_hash_index_find(Z_ARRVAL_PP(current_elem), 1, (void **)&y);
                        if (Z_TYPE_PP(x) != IS_LONG)
                            convert_to_long(*x);
                        if (Z_TYPE_PP(y) != IS_LONG)
                            convert_to_long(*y);

                        points[t].x = Z_LVAL_PP(x);
                        points[t].y = Z_LVAL_PP(y);
                        t++;
                        point_counts[ints_in_count]++;
                    }
                }
            }

            ints_in_count++;
        }
        else
        {
            php_error_docref(NULL TSRMLS_CC, E_ERROR,
                "expecting array for parameter %d, got %s", zend_zval_type_name(*(parameters[i])));
            goto CLEANUP;
        }
    }

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);
    RETURN_BOOL(PolyPolygon(dc_obj->hdc, points, point_counts, ints_in_count));

CLEANUP:
    efree(parameters);
    efree(points);
    efree(point_counts);
}

PHP_METHOD(WinGdiPath, roundedRectangle)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;
    int x1, y1, x2, y2,
        width, height;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llllll", 
            &x1, &y1, &x2, &y2, &width, &height) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = zend_object_store_get_object(getThis() TSRMLS_CC);
    dc_obj   = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);

    RETURN_BOOL(RoundRect(dc_obj->hdc, x1, y1, x2, y2, width, height));
}

// Now for the Line & Curve functions

PHP_METHOD(WinGdiPath, angleArc)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;
    DWORD radius;
    FLOAT start, sweep; 
    int x, y;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llldd", &x, &y, &radius, &start, &sweep) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);

    RETURN_BOOL(AngleArc(dc_obj->hdc, x, y, radius, start, sweep));
}

PHP_METHOD(WinGdiPath, arc)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;
    int x1, y1, x2, y2, x3, y3, x4, y4;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llllllll",
            &x1, &y1, &x2, &y2, &x3, &y3, &x4, &y4) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);

    RETURN_BOOL(Arc(dc_obj->hdc, x1, y1, x2, y2, x3, y3, x4, y4));
}

PHP_METHOD(WinGdiPath, arcTo)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;
    int x1, y1, x2, y2, x3, y3, x4, y4;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llllllll",
            &x1, &y1, &x2, &y2, &x3, &y3, &x4, &y4) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);

    RETURN_BOOL(Arc(dc_obj->hdc, x1, y1, x2, y2, x3, y3, x4, y4));
}

PHP_METHOD(WinGdiPath, getArcDirection)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters_none() == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);

    RETURN_LONG(GetArcDirection(dc_obj->hdc));
}

PHP_METHOD(WinGdiPath, lineTo)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;
    zval ***parameters,
         **x, **y;
    POINT *points = NULL;
    DWORD points_total = 0;
    int param_count, i;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &parameters, &param_count) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);
    points = emalloc(param_count * sizeof(POINT));

    for (i = 0; i < param_count; i++)
    {
        // We expect only arrays
        if (Z_TYPE_PP(parameters[i]) != IS_ARRAY) 
        {
            php_error_docref(NULL TSRMLS_CC, E_ERROR, "expected array for parameter %d, got %s",
                i + 1, zend_zval_type_name(*(parameters[i])));
            goto CLEANUP;
        }
        else
        {
            // We want 2 elements
            if (zend_hash_num_elements(Z_ARRVAL_PP(parameters[i])) != 2)
            {
                php_error_docref(NULL TSRMLS_CC, E_ERROR, 
                    "expected 2 elements for array at parameter %d, got %d", 
                    i + 1, zend_hash_num_elements(Z_ARRVAL_PP(parameters[i])));
                goto CLEANUP;
            }
            else
            {
                zend_hash_index_find(Z_ARRVAL_PP(parameters[i]), 0, (void **)&x);
                zend_hash_index_find(Z_ARRVAL_PP(parameters[i]), 1, (void **)&y);
                if (Z_TYPE_PP(x) != IS_LONG) convert_to_long(*x);
                if (Z_TYPE_PP(y) != IS_LONG) convert_to_long(*y);
                points[i].x = Z_LVAL_PP(x);
                points[i].y = Z_LVAL_PP(y);
                points_total++;
            }
        }
    }

    RETVAL_BOOL(PolylineTo(dc_obj->hdc, points, points_total));

CLEANUP:
    efree(points);
}

PHP_METHOD(WinGdiPath, moveTo)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;
    POINT previous;
    BOOL result;
    zval *out_zval = NULL;
    int x, y;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll|z", &x, &y, &out_zval) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);

    result = MoveToEx(dc_obj->hdc, x, y, (out_zval) ? &previous : NULL);
    if (out_zval)
    {
        zval_dtor(out_zval);
        array_init(out_zval);
        add_next_index_long(out_zval, previous.x);
        add_next_index_long(out_zval, previous.y);
    }
    
    RETURN_BOOL(result);
}

PHP_METHOD(WinGdiPath, beizer)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;
    zval ***parameters,
         **x, **y;
    POINT *points = NULL;
    DWORD points_total = 0;
    int param_count, i;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &parameters, &param_count) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);
    points = emalloc(param_count * sizeof(POINT));

    for (i = 0; i < param_count; i++)
    {
        // We expect only arrays
        if (Z_TYPE_PP(parameters[i]) != IS_ARRAY) 
        {
            php_error_docref(NULL TSRMLS_CC, E_ERROR, "expected array for parameter %d, got %s",
                i + 1, zend_zval_type_name(*(parameters[i])));
            goto CLEANUP;
        }
        else
        {
            // We want 2 elements
            if (zend_hash_num_elements(Z_ARRVAL_PP(parameters[i])) != 2)
            {
                php_error_docref(NULL TSRMLS_CC, E_ERROR, 
                    "expected 2 elements for array at parameter %d, got %d", 
                    i + 1, zend_hash_num_elements(Z_ARRVAL_PP(parameters[i])));
                goto CLEANUP;
            }
            else
            {
                zend_hash_index_find(Z_ARRVAL_PP(parameters[i]), 0, (void **)&x);
                zend_hash_index_find(Z_ARRVAL_PP(parameters[i]), 1, (void **)&y);
                if (Z_TYPE_PP(x) != IS_LONG) convert_to_long(*x);
                if (Z_TYPE_PP(y) != IS_LONG) convert_to_long(*y);
                points[i].x = Z_LVAL_PP(x);
                points[i].y = Z_LVAL_PP(y);
                points_total++;
            }
        }
    }

    RETVAL_BOOL(PolyBezier(dc_obj->hdc, points, points_total));

CLEANUP:
    efree(points);
}

PHP_METHOD(WinGdiPath, beizerTo)
{    
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;
    zval ***parameters,
         **x, **y;
    POINT *points = NULL;
    DWORD points_total = 0;
    int param_count, i;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &parameters, &param_count) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);
    points = emalloc(param_count * sizeof(POINT));

    for (i = 0; i < param_count; i++)
    {
        // We expect only arrays
        if (Z_TYPE_PP(parameters[i]) != IS_ARRAY) 
        {
            php_error_docref(NULL TSRMLS_CC, E_ERROR, "expected array for parameter %d, got %s",
                i + 1, zend_zval_type_name(*(parameters[i])));
            goto CLEANUP;
        }
        else
        {
            // We want 2 elements
            if (zend_hash_num_elements(Z_ARRVAL_PP(parameters[i])) != 2)
            {
                php_error_docref(NULL TSRMLS_CC, E_ERROR, 
                    "expected 2 elements for array at parameter %d, got %d", 
                    i + 1, zend_hash_num_elements(Z_ARRVAL_PP(parameters[i])));
                goto CLEANUP;
            }
            else
            {
                zend_hash_index_find(Z_ARRVAL_PP(parameters[i]), 0, (void **)&x);
                zend_hash_index_find(Z_ARRVAL_PP(parameters[i]), 1, (void **)&y);
                if (Z_TYPE_PP(x) != IS_LONG) convert_to_long(*x);
                if (Z_TYPE_PP(y) != IS_LONG) convert_to_long(*y);
                points[i].x = Z_LVAL_PP(x);
                points[i].y = Z_LVAL_PP(y);
                points_total++;
            }
        }
    }

    RETVAL_BOOL(PolyBezierTo(dc_obj->hdc, points, points_total));

CLEANUP:
    efree(points);
}

PHP_METHOD(WinGdiPath, draw)
{    
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;
    zval ***parameters,
         **x, **y, **type;
    POINT *points = NULL;
    BYTE  *types = NULL;
    int points_total = 0;
    int param_count, i;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &parameters, &param_count) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);
    points = emalloc(param_count * sizeof(POINT));
    types = emalloc(param_count * sizeof(BYTE));

    for (i = 0; i < param_count; i++)
    {
        // We expect only arrays
        if (Z_TYPE_PP(parameters[i]) != IS_ARRAY) 
        {
            php_error_docref(NULL TSRMLS_CC, E_ERROR, "expected array for parameter %d, got %s",
                i + 1, zend_zval_type_name(*(parameters[i])));
            goto CLEANUP;
        }
        else
        {
            // We want 3 elements
            if (zend_hash_num_elements(Z_ARRVAL_PP(parameters[i])) != 3)
            {
                php_error_docref(NULL TSRMLS_CC, E_ERROR, 
                    "expected 3 elements for array at parameter %d, got %d", 
                    i + 1, zend_hash_num_elements(Z_ARRVAL_PP(parameters[i])));
                goto CLEANUP;
            }
            else
            {
                zend_hash_index_find(Z_ARRVAL_PP(parameters[i]), 0, (void **)&x);
                zend_hash_index_find(Z_ARRVAL_PP(parameters[i]), 1, (void **)&y);
                zend_hash_index_find(Z_ARRVAL_PP(parameters[i]), 2, (void **)&type);
                if (Z_TYPE_PP(x) != IS_LONG) convert_to_long(*x);
                if (Z_TYPE_PP(y) != IS_LONG) convert_to_long(*y);
                if (Z_TYPE_PP(type) != IS_LONG) convert_to_long(*type);
                points[i].x = Z_LVAL_PP(x);
                points[i].y = Z_LVAL_PP(y);
                types[i] = (BYTE)Z_LVAL_PP(type);
                points_total++;
            }
        }
    }

    RETVAL_BOOL(PolyDraw(dc_obj->hdc, points, types, points_total));

CLEANUP:
    efree(points);
    efree(types);
}

PHP_METHOD(WinGdiPath, line)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    POINT *points = NULL;
    DWORD *point_counts = NULL;
    zval ***parameters,
         **x, **y,
         **current_elem;
    HashTable *current_param;
    HashPointer p;
    int  param_count  = 0,
         total_points = 0, // The amount of POINTs in points
         i;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &parameters, &param_count) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    for (i = 0; i < param_count; i++)
    {
        // All parameters should be arrays
        if (Z_TYPE_PP(parameters[i]) == IS_ARRAY)
        {
            // Add room for new DWORD
            point_counts = erealloc(point_counts, (i + 1) * sizeof(DWORD));
            // Initialise number of points for current array
            point_counts[i] = 0;
            // Less typing
            current_param = Z_ARRVAL_PP(parameters[i]);

            // Loop over all elements in current array parameter
            for (zend_hash_internal_pointer_reset(current_param);
                 zend_hash_has_more_elements(current_param) == SUCCESS;
                 zend_hash_move_forward(current_param))
            {
                // Add room for new POINT
                points = erealloc(points, (total_points + 1) * sizeof(POINT));
                // We want the hash pointer so we know what index we're at currently
                zend_hash_get_pointer(current_param, &p);
                // Pull the current data from hashtable
                zend_hash_get_current_data(current_param, (void **)&current_elem);

                // Sanity checks:
                // 1) An array
                // 2) 2 elements
                if (Z_TYPE_PP(current_elem) != IS_ARRAY)
                {
                    php_error_docref(NULL TSRMLS_CC, E_ERROR,
                        "expected point-array for parameter %d, index %d, got %s",
                        i + 1, p.h, zend_zval_type_name(*current_elem));
                    goto CLEANUP;
                }
                if (zend_hash_num_elements(Z_ARRVAL_PP(current_elem)) != 2)
                {
                    php_error_docref(NULL TSRMLS_CC, E_ERROR,
                        "expected point-array for parameter %d index %d to have exactly 2 elements, %d given",
                        i + 1, p.h, zend_hash_num_elements(Z_ARRVAL_PP(current_elem)));
                    goto CLEANUP;
                }

                // Grab x and y coord and convert to long if needed
                zend_hash_index_find(Z_ARRVAL_PP(current_elem), 0, (void **)&x);
                zend_hash_index_find(Z_ARRVAL_PP(current_elem), 1, (void **)&y);
                if (Z_TYPE_PP(x) != IS_LONG) convert_to_long(*x);
                if (Z_TYPE_PP(y) != IS_LONG) convert_to_long(*y);
                // Store
                points[total_points].x = Z_LVAL_PP(x);
                points[total_points].y = Z_LVAL_PP(y);
                total_points++;
                point_counts[i]++;
            }
        }
        else
        {
            php_error_docref(NULL TSRMLS_CC, E_ERROR, 
                "expected array for parameter %d, got %s", i + 1, zend_zval_type_name(*(parameters[i])));
            goto CLEANUP;
        }
    }

    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);
    RETURN_BOOL(PolyPolyline(dc_obj->hdc, points, point_counts, total_points));

CLEANUP:
    efree(points);
    efree(point_counts);
}

PHP_METHOD(WinGdiPath, setArcDirection)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_path_object *path_obj;
    int arc_dir;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &arc_dir) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    path_obj = wingdi_path_object_get(getThis() TSRMLS_CC);
    dc_obj = wingdi_devicecontext_object_get(path_obj->device_context TSRMLS_CC);

    RETURN_BOOL(SetArcDirection(dc_obj->hdc, arc_dir));
}

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