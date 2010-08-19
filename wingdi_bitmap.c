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

/* Custom Object junk */
zend_class_entry *ce_wingdi_bitmap;

void wingdi_bitmap_destructor(void *object TSRMLS_DC);

/* ----------------------------------------------------------------
  Win\Gdi\Bitmap Userland API                                                      
------------------------------------------------------------------*/

ZEND_BEGIN_ARG_INFO_EX(WinGdiBitmap_load_args, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, width)
	ZEND_ARG_INFO(0, height)
ZEND_END_ARG_INFO()

/* {{{ proto object Win\Gdi\Bitmap::load(string file[, int width, int height)
       takes a filename and loads it into a bitmap - note this does NOT work with
	   PHP streams, a version switching out on createbitmap could be used at some point */
PHP_METHOD(WinGdiBitmap, load)
{
	HANDLE bitmap;
	char * filename;
	int width = 0, height = 0, type = LR_LOADFROMFILE, str_len;
	wingdi_bitmap_object *bitmap_object;

    WINGDI_ERROR_HANDLING()
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|ll", &filename, &str_len, &width, &height) == FAILURE) {
		return;
	}
    WINGDI_RESTORE_ERRORS()

	/* if width and height are 0, we assume you want default */
	if (width == 0 && height == 0) {
		type |= LR_DEFAULTSIZE;
	}
	bitmap = LoadImage(NULL, (LPCTSTR) filename, IMAGE_BITMAP, width, height, type);
	if (bitmap == NULL) {
		wingdi_create_error(GetLastError(), ce_wingdi_exception TSRMLS_CC);
		return;
	}
	object_init_ex(return_value, ce_wingdi_bitmap);
	bitmap_object = (wingdi_bitmap_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	bitmap_object->bitmap_handle = bitmap;
}
/* }}} */

/* regular Bitmap methods */
static const zend_function_entry wingdi_bitmap_functions[] = {
	PHP_ME(WinGdiBitmap, load, WinGdiBitmap_load_args, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

/* ----------------------------------------------------------------
  Win\Gdi\Bitmap Custom Object magic                                                 
------------------------------------------------------------------*/

/* {{{ wingdi_bitmap_object_new
       creates a new bitmap object */
zend_object_value wingdi_bitmap_object_new(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value retval;
	zval *tmp;
	wingdi_bitmap_object *object;

	object = emalloc(sizeof(wingdi_bitmap_object));
	object->std.ce = ce;
	retval.handle = zend_objects_store_put(object, (zend_objects_store_dtor_t) zend_objects_destroy_object, (zend_objects_free_object_storage_t) wingdi_bitmap_destructor, NULL TSRMLS_CC);
	retval.handlers = zend_get_std_object_handlers();
	object->handle = retval.handle;
	object->std.guards = NULL;
	object->bitmap_handle = NULL;
       
	ALLOC_HASHTABLE(object->std.properties);
	zend_hash_init(object->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
	zend_hash_copy(object->std.properties, &ce->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));

	return retval;
}
/* }}} */

/* cleans up the object */
void wingdi_bitmap_destructor(void *object TSRMLS_DC)
{
	wingdi_bitmap_object *bitmap_object = (wingdi_bitmap_object *)object;

	if (bitmap_object->bitmap_handle) {
		DeleteObject(bitmap_object->bitmap_handle);
	}

	zend_hash_destroy(bitmap_object->std.properties);
	FREE_HASHTABLE(bitmap_object->std.properties);
	
	efree(object);
}

/* ----------------------------------------------------------------
  Win\Gdi\Bitmap LifeCycle Functions                                                    
------------------------------------------------------------------*/

PHP_MINIT_FUNCTION(wingdi_bitmap)
{
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_NS, "Bitmap", wingdi_bitmap_functions);
	ce_wingdi_bitmap = zend_register_internal_class(&ce TSRMLS_CC);
	ce_wingdi_bitmap->create_object  = wingdi_bitmap_object_new;

	return SUCCESS;
}