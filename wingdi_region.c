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

zend_class_entry *ce_wingdi_region,
                 *ce_wingdi_region_combine;

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
	ZEND_ARG_OBJ_INFO(1, display_context, Win\\Gdi\\DeviceContext, 0)
	ZEND_ARG_OBJ_INFO(1, region, Win\\Gdi\\Region, 0)
	ZEND_ARG_OBJ_INFO(1, brush, Win\\Gdi\\Brush, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_region_get_box, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_region_frame, 0, 0, 4)
	ZEND_ARG_OBJ_INFO(1, display_context, Win\\Gdi\\DeviceContext, 0)
	ZEND_ARG_OBJ_INFO(1, brush, Win\\Gdi\\Brush, 0)
	ZEND_ARG_INFO(0, width)
	ZEND_ARG_INFO(0, height)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_region_invert, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(1, display_context, Win\\Gdi\\DeviceContext, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_region_offset, 0, 0, 2)
	ZEND_ARG_INFO(0, x)
	ZEND_ARG_INFO(0, y)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_wingdi_region_paint, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(1, display_context, Win\\Gdi\\DeviceContext, 0)
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
	
	WINGDI_ERROR_HANDLING()
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &reg_a_zval, &reg_b_zval) == FAILURE)
		return;
	WINGDI_RESTORE_ERRORS()

    reg_obj_a = (wingdi_region_object *)wingdi_region_object_get(reg_a_zval TSRMLS_CC);
	reg_obj_b = (wingdi_region_object *)wingdi_region_object_get(reg_b_zval TSRMLS_CC);

	result = EqualRgn(reg_obj_a->region_handle, reg_obj_b->region_handle);
	RETURN_BOOL(result);
}

/* {{{ proto void Win\Gdi\Region::combine(Win\Gdi\Region dest, Win\Gdi\Region source1, Win\Gdi\Region source2, int mode)
       Combines two regions into the destination region. The two regions are combined according
       to the specified mode.
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
	
	WINGDI_ERROR_HANDLING()
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzzl", &reg_out_zval, &reg_a_zval, &reg_b_zval, &mode) == FAILURE)
	{
		return;
	}
	WINGDI_RESTORE_ERRORS()

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

/* {{{ proto bool Win\Gdi\Region->fill(Win\Gdi\DeviceContext dc, Win\Gdi\Brush brush)
	   Fills a region, using the specified brush.
*/
PHP_METHOD(WinGdiRegion, fill)
{
	wingdi_devicecontext_object *dc_obj;
	wingdi_region_object *reg_obj;
	wingdi_brush_object  *brush_obj;
		zval *dc_zval,
		 *brush_zval;
	BOOL result;

	WINGDI_ERROR_HANDLING()
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &dc_zval, &brush_zval) == FAILURE)
		return;
	WINGDI_RESTORE_ERRORS()

	dc_obj    = wingdi_devicecontext_object_get(dc_zval TSRMLS_CC);
	reg_obj   = wingdi_region_object_get(getThis() TSRMLS_CC);
	brush_obj = wingdi_brush_object_get(brush_zval TSRMLS_CC);

	result = FillRgn(dc_obj->hdc, reg_obj->region_handle, brush_obj->brush_handle);

	RETURN_BOOL(result);
}
/* }}} */

/* {{{ proto bool Win\Gdi\Region->frame(Win\Gdi\DeviceContext dc, Win\Gdi\Brush brush, int width, int height)
       Draws a border around specified region using the specified brush
*/
PHP_METHOD(WinGdiRegion, frame)
{
    wingdi_devicecontext_object *dc_obj;
    wingdi_region_object *reg_obj = wingdi_region_object_get(getThis() TSRMLS_CC);
    wingdi_brush_object  *br_obj;
    zval *dc_zval,
         *br_zval;
    BOOL result;
    int width, 
        height;

    WINGDI_ERROR_HANDLING();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzll", &dc_zval, &br_zval, &width, &height) == FAILURE)
        return;
    WINGDI_RESTORE_ERRORS();

    dc_obj = wingdi_devicecontext_object_get(dc_zval TSRMLS_CC);
    br_obj = wingdi_brush_object_get(br_zval TSRMLS_CC);
    
    result = FrameRgn(dc_obj->hdc, reg_obj->region_handle, br_obj->brush_handle, width, height);
    RETURN_BOOL(result);
}

/* {{{ proto array Win\Gdi\Region->getBox()
	   Returns an array of the LPRECT data associated with the region.
*/
PHP_METHOD(WinGdiRegion, getBox)
{
		wingdi_region_object *reg_obj;
	LPRECT    box = malloc(sizeof(RECT));
	int       result;

	WINGDI_ERROR_HANDLING()
	if (zend_parse_parameters_none() == FAILURE)
		return;
	WINGDI_RESTORE_ERRORS()

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

/* {{{ proto bool Win\Gdi\Region->invert()
       Inverts a region's colors
*/
PHP_METHOD(WinGdiRegion, invert)
{
		wingdi_region_object *reg_obj;
	wingdi_devicecontext_object *dc_obj;
	zval *dc_zval;
	BOOL result;

	WINGDI_ERROR_HANDLING()
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &dc_zval) == FAILURE)
		return;
	WINGDI_RESTORE_ERRORS()

	reg_obj = wingdi_region_object_get(getThis() TSRMLS_CC);
	dc_obj  = wingdi_devicecontext_object_get(dc_zval TSRMLS_CC);

	result = InvertRgn(dc_obj->hdc, reg_obj->region_handle);

	RETURN_BOOL(result)
}
/* }}} */

/* {{{ proto int Win\Gdi\Region->offset(int x, int y)
       Moves a region by the specificed offsets
*/
PHP_METHOD(WinGdiRegion, offset)
{
		wingdi_region_object *reg_obj;
	int x_offset,
		y_offset,
		result;

	WINGDI_ERROR_HANDLING()
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &x_offset, &y_offset) == FAILURE)
		return;
	WINGDI_RESTORE_ERRORS()

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

/* {{{ proto bool Win\Gdi\Region->paint(Win\Gdi\DeviceContext dc)
       Paints the specified region by using the brush currently selected into
       the device context
*/
PHP_METHOD(WinGdiRegion, paint)
{
		wingdi_region_object *reg_obj;
	wingdi_devicecontext_object *dc_obj;
	zval *dc_zval;
	BOOL result;

	WINGDI_ERROR_HANDLING()
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &dc_zval) == FAILURE)
		return;
	WINGDI_RESTORE_ERRORS()

	reg_obj = wingdi_region_object_get(getThis() TSRMLS_CC);
	dc_obj  = wingdi_devicecontext_object_get(dc_zval TSRMLS_CC);

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
		wingdi_region_object *reg_obj;
	BOOL result;
	int  x, y;

	WINGDI_ERROR_HANDLING()
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &x, &y) == FAILURE)
		return;
	WINGDI_RESTORE_ERRORS()

	reg_obj = wingdi_region_object_get(getThis() TSRMLS_CC);

	result = PtInRegion(reg_obj->region_handle, x, y);
	RETURN_BOOL(result);
}
/* }}} */

/* {{{ proto bool Win\Gdi\Region->rectangleIn(array rect_coords)
       Determines whether any part of the specified rectangle is in the region
*/
PHP_METHOD(WinGdiRegion, rectangleIn)
{
		wingdi_region_object *reg_obj;
	zval *rect_zval;
	BOOL result;
	LONG *left, *top, *right, *bottom;
	RECT rect;

	WINGDI_ERROR_HANDLING()
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &rect_zval) == FAILURE)
		return;
	WINGDI_RESTORE_ERRORS()

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
/* }}} */

/* {{{ proto bool Win\Gdi\Region->setRectangle()
*/
PHP_METHOD(WinGdiRegion, setRectangle)
{
	/** Not sure how to implement this.
        Current thoughts are:
        1) static method that returns a new obj, based on old object, and
           nullify the object's region handle. Any attempts to use it there-
           after will fail. But then the bool returned by SetRectRgn is not 
           used.
        2) static method that returns the boolean result of SetRectRgn. The 
           method would take 2 objects, and perform similar operations on them
           as 1.
        3) the cleaner way would be to have an instance method and internally
           change the type of the object (getThis()), but I'm not sure how to 
           do that.
    */
}
/* }}} */

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
  Win\Gdi\Region LifeCycle Functions                                                    
------------------------------------------------------------------*/

/* {{{ PHP_MINIT_FUNCTION(wingdi_region) */
PHP_MINIT_FUNCTION(wingdi_region)
{
	zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_NS, "Region", wingdi_region_functions);
	ce_wingdi_region = zend_register_internal_class(&ce TSRMLS_CC);
	ce_wingdi_region->create_object = wingdi_region_object_new;

    INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_REGION_NS, "Combine", NULL); 
    ce_wingdi_region_combine = zend_register_internal_class_ex(&ce, ce_wingdi_region, PHP_WINGDI_REGION_NS TSRMLS_CC);
    ce_wingdi_region_combine->ce_flags |= ZEND_ACC_IMPLICIT_ABSTRACT_CLASS | ZEND_ACC_FINAL_CLASS;
    // PHP doesn't like constants using reserved keywords, so we'll prefix these.
    zend_declare_class_constant_long(ce_wingdi_region_combine, "RGN_COPY", sizeof("RGN_COPY") - 1, RGN_COPY TSRMLS_CC);
    zend_declare_class_constant_long(ce_wingdi_region_combine, "RGN_DIFF", sizeof("RGN_DIFF") - 1, RGN_DIFF TSRMLS_CC);
    zend_declare_class_constant_long(ce_wingdi_region_combine, "RGN_AND",  sizeof("RGN_AND") - 1,  RGN_AND  TSRMLS_CC);
    zend_declare_class_constant_long(ce_wingdi_region_combine, "RGN_XOR",  sizeof("RGN_XOR") - 1,  RGN_XOR  TSRMLS_CC);
    zend_declare_class_constant_long(ce_wingdi_region_combine, "RGN_OR",   sizeof("RGN_OR") -1,    RGN_OR   TSRMLS_CC);

    PHP_MINIT(wingdi_region_rectangle)(INIT_FUNC_ARGS_PASSTHRU);
    PHP_MINIT(wingdi_region_roundedrectangle)(INIT_FUNC_ARGS_PASSTHRU);
    PHP_MINIT(wingdi_region_elliptic)(INIT_FUNC_ARGS_PASSTHRU);
    PHP_MINIT(wingdi_region_polygon)(INIT_FUNC_ARGS_PASSTHRU);

	return SUCCESS;
}
/* }}} */
