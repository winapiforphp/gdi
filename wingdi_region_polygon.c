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

zend_class_entry *ce_wingdi_polygon_region,
                 *ce_wingdi_polygon_region_fill;

/* ----------------------------------------------------------------
  Win\Gdi\Region\Polygon Userland API                                                    
------------------------------------------------------------------*/

/* {{{ proto Win\Gdi\Region\Polygon Win\Gdi\Region\Polygon->__construct(array points1, [array points2, ...], int mode) 
       Creates a polygon region with the specified points
*/
PHP_METHOD(WinGdiRegionPolygon, __construct)
{
    wingdi_region_object *reg_obj;
    HashPointer p;              // We use this to determine what index we are at in the array
    POINT *points = NULL;
    zval  ***parameters,
          **current_elem,
          **x, **y;
    INT *point_counts   = NULL; // Number of POINTs in corresponding points array
    int  ints_in_count  = 0,    // Number of ints in point_counts
         poly_fill_mode = ALTERNATE,
         param_count,
         i, t = 0;              // Total number of POINTs

    WINGDI_ERROR_HANDLING();
    // Will throw an exception if no parameters are given
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &parameters, &param_count) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    // We need an array for the first parameter
    if (Z_TYPE_PP(parameters[0]) != IS_ARRAY)
    {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "expects an array for parameter 1");
        goto CLEANUP;
    }
    // Need an array or int for the last parameter
    if (Z_TYPE_PP(parameters[param_count - 1]) != IS_ARRAY &&
        Z_TYPE_PP(parameters[param_count - 1]) != IS_LONG)
    {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, 
            "expects an array or integer for last parameter, %s given", 
            zend_zval_type_name(*(parameters[param_count - 1])));
        goto CLEANUP;
    }

    // Loop over the given parameters
    for (i = 0; i < param_count; i++)
    {
        // I can has array
        if (Z_TYPE_PP(parameters[i]) == IS_ARRAY)
        {
            // Reallocate POINT count array to have space for new int
            point_counts = erealloc(point_counts, ints_in_count * sizeof(INT) + sizeof(INT));
            point_counts[ints_in_count] = 0;

            for (zend_hash_internal_pointer_reset(Z_ARRVAL_PP(parameters[i]));
                 zend_hash_has_more_elements(Z_ARRVAL_PP(parameters[i])) == SUCCESS;
                 zend_hash_move_forward(Z_ARRVAL_PP(parameters[i])))
            {
                // Reallocate POINT array to have space for new POINT
                points = erealloc(points, t * sizeof(POINT) + sizeof(POINT));
                zend_hash_get_pointer(Z_ARRVAL_PP(parameters[i]), &p);
                zend_hash_get_current_data(Z_ARRVAL_PP(parameters[i]), (void **)&current_elem);
                // We DEMAND an array!
                if (Z_TYPE_PP(current_elem) != IS_ARRAY)
                {
                    php_error_docref(NULL TSRMLS_CC, E_ERROR,
                        "expected array in array for parameter %d, got %s", i + 1, zend_zval_type_name(*current_elem));
                    goto CLEANUP;
                }
                else
                {
                    // And that array needs 2 elements
                    // And that array needs 2 elements!
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
                        // Increment to total counter for the POINTS
                        t++;
                        // And do the same for the current INT element
                        point_counts[ints_in_count]++;
                    }
                }
            }

            ints_in_count++;
        }
        // I can't has array, but has integer for last param instead! :D
        else if (Z_TYPE_PP(parameters[i]) == IS_LONG && i == param_count - 1)
        {
            poly_fill_mode = Z_LVAL_PP(parameters[i]);
            if (poly_fill_mode != ALTERNATE || poly_fill_mode != WINDING)
            {
                php_error_docref(NULL TSRMLS_CC, E_ERROR, 
                    "invalid value given for poly_fill_mode parameter");
                goto CLEANUP;
            }
            // No need to break from the loop, we're on the last iteration
        }
        // I can has useless programmer that can't read the documentation :(
        else
        {
            php_error_docref(NULL TSRMLS_CC, E_ERROR,
                "expecting array for parameter %d, got %s", zend_zval_type_name(*(parameters[i])));
            goto CLEANUP;
        }
    }

    reg_obj = zend_object_store_get_object(getThis() TSRMLS_CC);
    reg_obj->region_handle = CreatePolyPolygonRgn(points, point_counts, ints_in_count, poly_fill_mode);
    if (!reg_obj->region_handle)
    {
        wingdi_create_error(GetLastError(), ce_wingdi_exception TSRMLS_CC);
    }

CLEANUP:
    efree(parameters);
    efree(points);
    efree(point_counts);
}
    /* }}} */

    /* {{{ proto int Win\Gdi\Region\Polygon::getFillMode(Win\Gdi\DeviceContext dc)
           Gets the fill mode for functions that fill polygons
*/
PHP_METHOD(WinGdiRegionPolygon, getFillMode)
{
    wingdi_devicecontext_object *dc_obj;
    zval *dc_zval;
    int mode;

    WINGDI_ERROR_HANDLING()
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &dc_zval) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS()

    dc_obj = (wingdi_devicecontext_object *)wingdi_devicecontext_object_get(dc_zval TSRMLS_CC);
    mode = GetPolyFillMode(dc_obj->hdc);
    if (mode == 0)
    {
        wingdi_create_error(GetLastError(), ce_wingdi_exception TSRMLS_CC);
        return;
    }
    RETURN_LONG(mode);
}
/* }}} */

/* {{{ proto int Win\Gdi\Region\Polygon::setFillMode(DeviceContext dc, int mode)
       Sets the poly fill modefor functions that fill polygons
*/
PHP_METHOD(WinGdiRegionPolygon, setFillMode)
{
    wingdi_devicecontext_object *dc_obj;
    zval *dc_zval;
    int mode, result;

    WINGDI_ERROR_HANDLING()
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zl", &dc_zval, &mode) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS()

    dc_obj = (wingdi_devicecontext_object *)wingdi_devicecontext_object_get(dc_zval TSRMLS_CC);
    result = SetPolyFillMode(dc_obj->hdc, mode);
    if (result == 0)
    {
        wingdi_create_error(GetLastError(), ce_wingdi_exception TSRMLS_CC);
        return;
    }
}
/* }}} */

static const zend_function_entry wingdi_region_polygon_functions[] = {
    PHP_ME(WinGdiRegionPolygon, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(WinGdiRegionPolygon, setFillMode, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    {NULL, NULL, NULL}
};

/* ----------------------------------------------------------------
  Win\Gdi\Region\Polygon LifeCycle Functions                                                    
------------------------------------------------------------------*/

PHP_MINIT_FUNCTION(wingdi_region_polygon)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_REGION_NS, "Polygon", wingdi_region_polygon_functions);
    ce_wingdi_polygon_region =
        zend_register_internal_class_ex(&ce, ce_wingdi_region, PHP_WINGDI_REGION_NS TSRMLS_CC);
    ce_wingdi_polygon_region->create_object = wingdi_region_object_new;

    INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_REGION_POLY_NS, "Fill", NULL);
    ce_wingdi_polygon_region_fill = zend_register_internal_class_ex(&ce, ce_wingdi_polygon_region, PHP_WINGDI_REGION_POLY_NS TSRMLS_CC);
    ce_wingdi_polygon_region_fill->ce_flags |= ZEND_ACC_IMPLICIT_ABSTRACT_CLASS | ZEND_ACC_FINAL_CLASS;
    zend_declare_class_constant_long(ce_wingdi_polygon_region_fill, "ALTERNATE", sizeof("ALTERNATE") - 1, ALTERNATE TSRMLS_CC);
    zend_declare_class_constant_long(ce_wingdi_polygon_region_fill, "WINDING",   sizeof("WINDING")  - 1,  WINDING   TSRMLS_CC);

    return SUCCESS;
}