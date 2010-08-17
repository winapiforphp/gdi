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
  |         Mark Skilbeck <markskilbeck@php.net>                         |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#include "php_wingdi.h"
#include "zend_exceptions.h"

zend_class_entry *ce_wingdi_region;
zend_class_entry *ce_wingdi_rect_region;
zend_class_entry *ce_wingdi_round_region;
zend_class_entry *ce_wingdi_elliptic_region;
zend_class_entry *ce_wingdi_polygon_region;
void wingdi_region_object_destroy(void * object TSRMLS_DC);

/* {{{ zend_object_value wingdi_region_object_new(zend_class_entry *ce TSRMLS_DC)
	   Create new object and store it in objects store; boilerplate boredom.
*/
zend_object_value wingdi_region_object_new(zend_class_entry *ce TSRMLS_DC) 
{
	zend_object_value    ret;
	wingdi_region_object *reg;
	zval                 *tmp;

	reg = emalloc(sizeof(wingdi_region_object));
	reg->std.ce = ce;
	reg->handle = zend_objects_store_put(
		reg, 
		(zend_objects_store_dtor_t)zend_objects_destroy_object,
        (zend_objects_free_object_storage_t)wingdi_region_object_destroy,
		NULL
		TSRMLS_CC
	);
	reg->region_handle = NULL;

	ret.handle   = reg->handle;
	ret.handlers = zend_get_std_object_handlers();

	ALLOC_HASHTABLE(reg->std.properties);
	zend_hash_init(reg->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
	zend_hash_copy(reg->std.properties, &ce->default_properties, 
        (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));

	return ret;
}
/* }}} */

/* {{{ 
*/
void wingdi_region_object_destroy(void *object TSRMLS_DC)
{
	wingdi_region_object *reg = (wingdi_region_object *)object;

	if (reg->region_handle)
		DeleteObject(reg->region_handle);

	zend_hash_destroy(reg->std.properties);
	FREE_HASHTABLE(reg->std.properties);

	efree(reg);
}
/* }}} */

/* {{{ zend_object_value wingdi_region_object_clone(zval *zobj TSRMLS_DC)
*/
void wingdi_region_object_clone(zval *zobj TSRMLS_DC) {
}
/* }}} */

/* ----------------------------------------------------------------
  Win\Gdi\Region Userland API
-----------------------------------------------------------------*/

ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_region_combine, 0, 0, 3)
	ZEND_ARG_OBJ_INFO(1, region_a, Win\\Gdi\\Region, 0)
	ZEND_ARG_OBJ_INFO(1, region_b, Win\\Gdi\\Region, 0)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_region_equal, 0, 0, 2)
	ZEND_ARG_OBJ_INFO(1, region_a, Win\\Gdi\\Region, 0)
	ZEND_ARG_OBJ_INFO(1, region_b, Win\\Gdi\\Region, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_region_fill, 0, 0, 3)
	ZEND_ARG_OBJ_INFO(1, display_context, Win\\Gdi\\DisplayContext, 0)
	ZEND_ARG_OBJ_INFO(1, region, Win\\Gdi\\Region, 0)
	ZEND_ARG_OBJ_INFO(1, brush, Win\\Gdi\\Brush, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_region_get_box, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_region_frame, 0, 0, 4)
	ZEND_ARG_OBJ_INFO(1, display_context, Win\\Gdi\\DisplayContext, 0)
	ZEND_ARG_OBJ_INFO(1, brush, Win\\Gdi\\Brush, 0)
	ZEND_ARG_INFO(0, width)
	ZEND_ARG_INFO(0, height)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_region_invert, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(1, display_context, Win\\Gdi\\DisplayContext, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_region_offset, 0, 0, 2)
	ZEND_ARG_INFO(0, x)
	ZEND_ARG_INFO(0, y)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_region_paint, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(1, display_context, Win\\Gdi\\DisplayContext, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_region_point_in, 0, 0, 2)
	ZEND_ARG_INFO(0, x)
	ZEND_ARG_INFO(0, y)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_region_rect_in, 0, 0, 1)
	ZEND_ARG_ARRAY_INFO(0, rect, 0)
ZEND_END_ARG_INFO()

/* {{{ proto bool Win\Gdi\Region::equal(Win\Gdi\Region region_a, Win\Gdi\Region region_b)
	   Checks the two specified regions to determine whether they are identical. 
	   The function considers two regions identical if they are equal in size and shape.
*/
PHP_METHOD(WinGdiRegion, equal)
{
	zval *reg_a_zval,
		 *reg_b_zval;
	BOOL result;
	wingdi_region_object *reg_obj_a,
						 *reg_obj_b;
	zend_error_handling  error_handling;

	zend_replace_error_handling(EH_THROW, ce_wingdi_argexception, &error_handling TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &reg_a_zval, &reg_b_zval) == FAILURE)
		return;
	zend_restore_error_handling(&error_handling TSRMLS_CC);

	/*if (Z_TYPE_P(reg_a_zval) == IS_OBJECT && 
		instanceof_function(Z_OBJCE_P(reg_a_zval), ce_wingdi_region TSRMLS_CC))
	{*/
		reg_obj_a = (wingdi_region_object *)wingdi_region_object_get(reg_a_zval TSRMLS_CC);
	/*}
	else
	{
		zend_throw_exception(ce_wingdi_argexception, 
			"Win\\Gdi\\Region::equal() expects parameter 1 to be of type Win\\Gdi\\Region", 0 TSRMLS_CC);
		return;
	}

	if (Z_TYPE_P(reg_b_zval) == IS_OBJECT &&
		instanceof_function(Z_OBJCE_P(reg_b_zval), ce_wingdi_region TSRMLS_CC))
	{*/
		reg_obj_b = (wingdi_region_object *)wingdi_region_object_get(reg_b_zval TSRMLS_CC);
	/*}
	else 
	{
		zend_throw_exception(ce_wingdi_argexception,
			"Win\\Gdi\\Region::equal() expects parameter 2 to be of type Win\\Gdi\\Region", 0 TSRMLS_CC);
		return;
	}*/

	result = EqualRgn(reg_obj_a->region_handle, reg_obj_b->region_handle);

	RETURN_BOOL(result);
}

/* {{{ proto void Win\Gdi\Region::combine(Win\Gdi\Region dest, Win\Gdi\Region source1, Win\Gdi\Region source2, int mode)
*/
PHP_METHOD(WinGdiRegion, combine)
{
	zval *reg_out_zval,
         *reg_a_zval,
		 *reg_b_zval;
	int  mode,
		 result;
	wingdi_region_object *reg_obj_out,
                         *reg_obj_a,
						 *reg_obj_b;
	zend_error_handling  error_handling;

	zend_replace_error_handling(EH_THROW, ce_wingdi_argexception, &error_handling TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzzl", &reg_out_zval, &reg_a_zval, &reg_b_zval, &mode) == FAILURE)
	{
		return;
	}
	zend_restore_error_handling(&error_handling TSRMLS_CC);

    reg_obj_out = (wingdi_region_object *)wingdi_region_object_get(reg_out_zval TSRMLS_CC);
    reg_obj_a = (wingdi_region_object *)wingdi_region_object_get(reg_a_zval TSRMLS_CC);
	reg_obj_b = (wingdi_region_object *)wingdi_region_object_get(reg_b_zval TSRMLS_CC);

	result = 
		CombineRgn(reg_obj_out->region_handle, reg_obj_a->region_handle, reg_obj_b->region_handle, mode);
	if (result == ERROR)
	{
		wingdi_create_error(GetLastError(), ce_wingdi_exception TSRMLS_CC);
		return;
	}
}
/* }}} */

/* {{{ proto bool Win\Gdi\Region->fill(Win\Gdi\DisplayContext dc, Win\Gdi\Brush brush)
	   Fills a region, using the specified brush.
*/
PHP_METHOD(WinGdiRegion, fill)
{
	wingdi_displaycontext_object *dc_obj;
	wingdi_region_object *reg_obj;
	wingdi_brush_object  *brush_obj;
	zend_error_handling  error_handling;
	zval *dc_zval,
		 *brush_zval;
	BOOL result;

	zend_replace_error_handling(EH_THROW, ce_wingdi_argexception, &error_handling TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &dc_zval, &brush_zval) == FAILURE)
		return;
	zend_restore_error_handling(&error_handling TSRMLS_CC);

	dc_obj    = wingdi_displaycontext_object_get(dc_zval TSRMLS_CC);
	reg_obj   = wingdi_region_object_get(getThis() TSRMLS_CC);
	brush_obj = wingdi_brush_object_get(brush_zval TSRMLS_CC);

	result = FillRgn(dc_obj->hdc, reg_obj->region_handle, brush_obj->brush_handle);

	RETURN_BOOL(result);
}
/* }}} */

/* {{{ proto array Win\Gdi\Region->getBox()
	   Returns an array of the LPRECT data associated with the region.
*/
PHP_METHOD(WinGdiRegion, getBox)
{
	zend_error_handling  error_handling;
	wingdi_region_object *reg_obj;
	LPRECT    box = malloc(sizeof(RECT));
	int       result;

	zend_replace_error_handling(EH_THROW, ce_wingdi_argexception, &error_handling TSRMLS_CC);
	if (zend_parse_parameters_none() == FAILURE)
		return;
	zend_restore_error_handling(&error_handling TSRMLS_CC);

	reg_obj = wingdi_region_object_get(getThis() TSRMLS_CC);
	/** Should we do anything with the return of GetRgnBox(), other than check for an error? */
	result = GetRgnBox(reg_obj->region_handle, box);
	if (result == ERROR)
	{
		wingdi_create_error(GetLastError(), ce_wingdi_exception TSRMLS_CC);
		return;
	}

	array_init(return_value);
	add_index_long(return_value, 0, box->left);
	add_index_long(return_value, 1, box->top);
	add_index_long(return_value, 2, box->right);
	add_index_long(return_value, 3, box->bottom);

	free(box);
}
/* }}} */

/* {{{
*/
PHP_METHOD(WinGdiRegion, invert)
{
	zend_error_handling  error_handling;
	wingdi_region_object *reg_obj;
	wingdi_displaycontext_object *dc_obj;
	zval *dc_zval;
	BOOL result;

	zend_replace_error_handling(EH_THROW, ce_wingdi_argexception, &error_handling TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &dc_zval) == FAILURE)
		return;
	zend_restore_error_handling(&error_handling TSRMLS_CC);

	reg_obj = wingdi_region_object_get(getThis() TSRMLS_CC);
	dc_obj  = wingdi_displaycontext_object_get(dc_zval TSRMLS_CC);

	result = InvertRgn(dc_obj->hdc, reg_obj->region_handle);

	RETURN_BOOL(result)
}
/* }}} */

/* {{{ 
*/
PHP_METHOD(WinGdiRegion, offset)
{
	zend_error_handling  error_handling;
	wingdi_region_object *reg_obj;
	int x_offset,
		y_offset,
		result;

	zend_replace_error_handling(EH_THROW, ce_wingdi_argexception, &error_handling TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &x_offset, &y_offset) == FAILURE)
		return;
	zend_restore_error_handling(&error_handling TSRMLS_CC);

	reg_obj = wingdi_region_object_get(getThis() TSRMLS_CC);

	result = OffsetRgn(reg_obj->region_handle, x_offset, y_offset);
	if (result == ERROR)
	{
		wingdi_create_error(GetLastError(), ce_wingdi_exception TSRMLS_CC);
		return;
	}

	RETURN_LONG(result);
}
/* }}} */

/* {{{ 
*/
PHP_METHOD(WinGdiRegion, paint)
{
	zend_error_handling  error_handling;
	wingdi_region_object *reg_obj;
	wingdi_displaycontext_object *dc_obj;
	zval *dc_zval;
	BOOL result;

	zend_replace_error_handling(EH_THROW, ce_wingdi_argexception, &error_handling TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &dc_zval) == FAILURE)
		return;
	zend_restore_error_handling(&error_handling TSRMLS_CC);

	reg_obj = wingdi_region_object_get(getThis() TSRMLS_CC);
	dc_obj  = wingdi_displaycontext_object_get(dc_zval TSRMLS_CC);

	result = PaintRgn(dc_obj->hdc, reg_obj->region_handle);

	RETURN_BOOL(result);
}
/* }}} */

/* {{{ proto bool Win\Gdi\Region->pointIn(int x, int y)
	   Determines whether a given point exists in the region.
	   Alias: Win\Gdi\Region->hasPoint(int x, int y)
*/
PHP_METHOD(WinGdiRegion, pointIn)
{
	zend_error_handling  error_handling;
	wingdi_region_object *reg_obj;
	BOOL result;
	int  x, y;

	zend_replace_error_handling(EH_THROW, ce_wingdi_argexception, &error_handling TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &x, &y) == FAILURE)
		return;
	zend_restore_error_handling(&error_handling TSRMLS_CC);

	reg_obj = wingdi_region_object_get(getThis() TSRMLS_CC);

	result = PtInRegion(reg_obj->region_handle, x, y);
	RETURN_BOOL(result);
}
/* }}} */

/* {{{
*/
PHP_METHOD(WinGdiRegion, rectangleIn)
{
	zend_error_handling  error_handling;
	wingdi_region_object *reg_obj;
	zval *rect_zval;
	BOOL result;
	LONG *left, *top, *right, *bottom;
	RECT rect;

	zend_replace_error_handling(EH_THROW, ce_wingdi_argexception, &error_handling TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &rect_zval) == FAILURE)
		return;
	zend_restore_error_handling(&error_handling TSRMLS_CC);

	reg_obj = wingdi_region_object_get(getThis() TSRMLS_CC);
	zend_hash_index_find(Z_ARRVAL_P(rect_zval), 0, &left);
	zend_hash_index_find(Z_ARRVAL_P(rect_zval), 1, &top);
	zend_hash_index_find(Z_ARRVAL_P(rect_zval), 2, &right);
	zend_hash_index_find(Z_ARRVAL_P(rect_zval), 3, &bottom);
	rect.left   = *left;
	rect.top    = *top;
	rect.right  = *right;
	rect.bottom = *bottom;
	result = RectInRegion(reg_obj->region_handle, &rect);

	RETURN_BOOL(result)
}

/* {{{
*/
PHP_METHOD(WinGdiRegion, setRectangle)
{
	zend_error_handling  error_handling;
	zend_object_value    obj_val;
	wingdi_region_object *old_reg, *new_reg;
	zval *old_reg_zval;
	BOOL result;
	int left, top,
        right, bottom;

	zend_replace_error_handling(EH_THROW, ce_wingdi_argexception, &error_handling TSRMLS_CC);
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zllll", 
			&old_reg_zval, &left, &top, &right, &bottom) == FAILURE)
		return;
	zend_restore_error_handling(&error_handling TSRMLS_CC);

	old_reg = wingdi_region_object_get(old_reg_zval TSRMLS_CC);
	obj_val = wingdi_region_object_new(old_reg->std.ce TSRMLS_CC);
	new_reg = zend_object_store_get_object_by_handle(obj_val.handle TSRMLS_CC);
	new_reg->region_handle = old_reg->region_handle;
	/* Discard old object's handle - any attempts to use it will fale. */
	old_reg->region_handle = NULL;

	result = SetRectRgn(new_reg->region_handle, left, top, right, bottom);

	RETURN_BOOL(result);
}

static const zend_function_entry wingdi_region_functions[] = {
	ZEND_ME(WinGdiRegion, combine, arginfo_wingdi_region_combine, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(WinGdiRegion, equal, arginfo_wingdi_region_equal, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	ZEND_ME(WinGdiRegion, fill, arginfo_wingdi_region_fill, ZEND_ACC_PUBLIC)
	ZEND_ME(WinGdiRegion, getBox, arginfo_wingdi_region_get_box, ZEND_ACC_PUBLIC)
	ZEND_ME(WinGdiRegion, invert, arginfo_wingdi_region_invert, ZEND_ACC_PUBLIC)
	ZEND_ME(WinGdiRegion, offset, arginfo_wingdi_region_offset, ZEND_ACC_PUBLIC)
	ZEND_ME(WinGdiRegion, paint, arginfo_wingdi_region_paint, ZEND_ACC_PUBLIC)
	ZEND_ME(WinGdiRegion, pointIn, arginfo_wingdi_region_point_in, ZEND_ACC_PUBLIC)
	ZEND_ME(WinGdiRegion, rectangleIn, arginfo_wingdi_region_rect_in, ZEND_ACC_PUBLIC)
	/* Aliases */
	ZEND_MALIAS(WinGdiRegion, harvester, combine, arginfo_wingdi_region_combine, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	/* Win\Gdi\Region->pointIn() doesn't read well (or make sense) to me. ->hasPoint() is much more intelligent. */
	ZEND_MALIAS(WinGdiRegion, hasPoint, pointIn, arginfo_wingdi_region_point_in, ZEND_ACC_PUBLIC)
	/* Likewise... */
	ZEND_MALIAS(WinGdiRegion, hasRectangle, rectangleIn, arginfo_wingdi_region_rect_in, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

/* ----------------------------------------------------------------
  Win\Gdi\Region\Rectangle Userland API                                                    
------------------------------------------------------------------*/

ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_rect_region, 0, 0, 4)
    ZEND_ARG_INFO(0, left)
    ZEND_ARG_INFO(0, top)
    ZEND_ARG_INFO(0, right)
    ZEND_ARG_INFO(0, bottom)
ZEND_END_ARG_INFO()

PHP_METHOD(WinGdiRegionRect, __construct)
{
    zend_error_handling  error_handling;
    wingdi_region_object *reg_obj;
    int left, top,
        right, bottom;

    zend_replace_error_handling(EH_THROW, ce_wingdi_argexception, &error_handling TSRMLS_CC);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llll", &left, &top, &right, &bottom) == FAILURE)
        return;
    zend_restore_error_handling(&error_handling TSRMLS_CC);

    reg_obj = wingdi_region_object_get(getThis() TSRMLS_CC);
    reg_obj->region_handle = CreateRectRgn(left, top, right, bottom);
}

static const zend_function_entry wingdi_region_rect_functions[] = {
    PHP_ME(WinGdiRegionRect, __construct, arginfo_wingdi_rect_region, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    {NULL, NULL, NULL}
};

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

PHP_METHOD(WinGdiRegionRoundedRect, __construct)
{
    zend_error_handling  error_handling;
    wingdi_region_object *reg_obj;
    int left, top,
        right, bottom,
        width, height;

    zend_replace_error_handling(EH_THROW, ce_wingdi_argexception, &error_handling TSRMLS_CC);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llllll", 
            &left, &top, &right, &bottom, &width, &height) == FAILURE)
        return;
    zend_restore_error_handling(&error_handling TSRMLS_CC);

    reg_obj = (wingdi_region_object *)wingdi_region_object_get(getThis() TSRMLS_CC);
    reg_obj->region_handle = CreateRoundRectRgn(left, top, right, bottom, width, height);
    if (reg_obj->region_handle == NULL)
    {
        wingdi_create_error(GetLastError(), ce_wingdi_exception TSRMLS_CC);
        return;
    }
}

static const zend_function_entry wingdi_region_rounded_rect_functions[] = {
    PHP_ME(WinGdiRegionRoundedRect, __construct, arginfo_wingdi_rounded_rect_region, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    {NULL, NULL, NULL}
};

/* ----------------------------------------------------------------
  Win\Gdi\Region\Elliptic Userland API                                                    
------------------------------------------------------------------*/

ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_elliptic_region, 0, 0, 4)
    ZEND_ARG_INFO(0, left)
    ZEND_ARG_INFO(0, top)
    ZEND_ARG_INFO(0, right)
    ZEND_ARG_INFO(0, bottom)
ZEND_END_ARG_INFO()

PHP_METHOD(WinGdiRegionElliptic, __construct)
{
    zend_error_handling error_handling;
    wingdi_region_object *reg_obj;
    int left, top,
        right, bottom;

    zend_replace_error_handling(EH_THROW, ce_wingdi_argexception, &error_handling TSRMLS_CC);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llll", &left, &top, &right, &bottom) == FAILURE)
        return;
    zend_restore_error_handling(&error_handling TSRMLS_CC);

    reg_obj = (wingdi_region_object *)wingdi_region_object_get(getThis() TSRMLS_CC);
    reg_obj->region_handle = CreateEllipticRgn(left, top, right, bottom);
    if (reg_obj->region_handle == NULL)
    {
        wingdi_create_error(GetLastError(), ce_wingdi_exception TSRMLS_CC);
        return;
    }
}


static const zend_function_entry wingdi_region_elliptic_functions[] = {
    PHP_ME(WinGdiRegionElliptic, __construct, arginfo_wingdi_elliptic_region, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    {NULL, NULL, NULL}
};

/* ----------------------------------------------------------------
  Win\Gdi\Region\Polygon Userland API                                                    
------------------------------------------------------------------*/

PHP_METHOD(WinGdiRegionPolygon, __construct)
{
    zend_error_handling  error_handling;
    wingdi_region_object *reg_obj;
    zval ***args, *arg, **ppzval, **xppzval, **yppzval;
    POINT *points;
    int narg = ZEND_NUM_ARGS(),
        i = 0, n = 0, mode = ALTERNATE;

    zend_replace_error_handling(EH_THROW, ce_wingdi_argexception, &error_handling TSRMLS_CC);
    if (narg > 1)
    {
        args = (zval ***)safe_emalloc(narg, sizeof(zval **), 0);
        if (zend_get_parameters_array_ex(narg, args) == FAILURE)
            /** Do we need to do an error here? */
            efree(args);
    }
    else
    {
        if (zend_parse_parameters(narg TSRMLS_CC, "a|l", &arg, &mode) == FAILURE)
            return;
    }
    zend_restore_error_handling(&error_handling TSRMLS_CC);

    reg_obj = (wingdi_region_object *)wingdi_region_object_get(getThis() TSRMLS_CC);

    if (narg > 1)
    {
        for (i = 0; i < narg; i++)
        {
        }
    }
    else
    {
        points = emalloc(narg * sizeof(POINT));
        for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(arg));
             zend_hash_has_more_elements(Z_ARRVAL_P(arg)) == SUCCESS;
             zend_hash_move_forward(Z_ARRVAL_P(arg)))
        {
            if (zend_hash_get_current_data(Z_ARRVAL_P(arg), (void **)&ppzval) == FAILURE)
                /** What to do here? */
                continue;

            /** If we do not have an array, give a warning and move onto next array. */
            if (Z_TYPE_PP(ppzval) != IS_ARRAY) 
            {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, 
                    "expects an array of arrays."
                    " At index %d something other than an array was given.",
                    n++
                );
                continue;
            }

            if (zend_hash_index_exists(Z_ARRVAL_PP(ppzval), 0) &&
                zend_hash_index_exists(Z_ARRVAL_PP(ppzval), 1))
            {
                zend_hash_index_find(Z_ARRVAL_PP(ppzval), 0, (void **)&xppzval);
                zend_hash_index_find(Z_ARRVAL_PP(ppzval), 1, (void **)&yppzval);

                if (Z_TYPE_PP(xppzval) != IS_LONG)
                    convert_to_long(*xppzval);
                if (Z_TYPE_PP(yppzval) != IS_LONG)
                    convert_to_long(*yppzval);
                
                points[i].x = Z_LVAL_PP(xppzval);
                points[i].y = Z_LVAL_PP(yppzval);
                i++;
            }
            else
            {
                php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "expects an array with 2 integers.");
                n++;
                continue;
            }
            n++;
        }
        reg_obj->region_handle = CreatePolygonRgn(points, i + 1, mode);
        if (reg_obj->region_handle == NULL)
        {
            wingdi_create_error(GetLastError(), ce_wingdi_exception TSRMLS_CC);
            efree(points);
            return;
        }
        efree(points);
    }
}

PHP_METHOD(WinGdiRegionPolygon, getFillMode)
{
    zend_error_handling error_handling;
    wingdi_displaycontext_object *dc_obj;
    zval *dc_zval;
    int mode;

    zend_replace_error_handling(EH_THROW, ce_wingdi_argexception, &error_handling TSRMLS_CC);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &dc_zval) == FAILURE)
        return;
    zend_restore_error_handling(&error_handling TSRMLS_CC);

    dc_obj = (wingdi_displaycontext_object *)wingdi_displaycontext_object_get(dc_zval TSRMLS_CC);
    mode = GetPolyFillMode(dc_obj->hdc);
    if (mode == 0)
    {
        wingdi_create_error(GetLastError(), ce_wingdi_exception TSRMLS_CC);
        return;
    }
}
PHP_METHOD(WinGdiRegionPolygon, setFillMode)
{
    zend_error_handling  error_handling;
    wingdi_displaycontext_object *dc_obj;
    zval *dc_zval;
    int mode, result;

    zend_replace_error_handling(EH_THROW, ce_wingdi_argexception, &error_handling TSRMLS_CC);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zl", &dc_zval, &mode) == FAILURE)
        return;
    zend_restore_error_handling(&error_handling TSRMLS_CC);

    dc_obj = (wingdi_displaycontext_object *)wingdi_displaycontext_object_get(dc_zval TSRMLS_CC);
    result = SetPolyFillMode(dc_obj->hdc, mode);
    if (result == 0)
    {
        wingdi_create_error(GetLastError(), ce_wingdi_exception TSRMLS_CC);
        return;
    }
}
static const zend_function_entry wingdi_region_polygon_functions[] = {
    PHP_ME(WinGdiRegionPolygon, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(WinGdiRegionPolygon, setFillMode, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    {NULL, NULL, NULL}
};

/* ----------------------------------------------------------------
  Win\Gdi\Region LifeCycle Functions                                                    
------------------------------------------------------------------*/

/* {{{ PHP_MINIT_FUNCTION(wingdi_region) */
PHP_MINIT_FUNCTION(wingdi_region)
{
	zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_NS, "Region", wingdi_region_functions);
	ce_wingdi_region = zend_register_internal_class(&ce TSRMLS_CC);
	ce_wingdi_region->create_object = wingdi_region_object_new;

    INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_REGION_NS, "Rectangle", wingdi_region_rect_functions);
    ce_wingdi_rect_region = 
        zend_register_internal_class_ex(&ce, ce_wingdi_region, PHP_WINGDI_REGION_NS TSRMLS_CC);
    ce_wingdi_rect_region->create_object = wingdi_region_object_new;

    INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_REGION_NS, "RoundedRectangle", wingdi_region_rounded_rect_functions);
    ce_wingdi_round_region =
        zend_register_internal_class_ex(&ce, ce_wingdi_region, PHP_WINGDI_REGION_NS TSRMLS_CC);
    ce_wingdi_round_region->create_object = wingdi_region_object_new;

    INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_REGION_NS, "Elliptic", wingdi_region_elliptic_functions);
    ce_wingdi_elliptic_region = 
        zend_register_internal_class_ex(&ce, ce_wingdi_region, PHP_WINGDI_REGION_NS TSRMLS_CC);
    ce_wingdi_elliptic_region->create_object = wingdi_region_object_new;

    INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_REGION_NS, "Polygon", wingdi_region_polygon_functions);
    ce_wingdi_polygon_region =
        zend_register_internal_class_ex(&ce, ce_wingdi_region, PHP_WINGDI_REGION_NS TSRMLS_CC);
    ce_wingdi_polygon_region->create_object = wingdi_region_object_new;

	return SUCCESS;
}
/* }}} */
