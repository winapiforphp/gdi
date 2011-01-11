/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2011 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Elizabeth Smith <auroraeosrose@php.net>                      |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#include "php_wingdi.h"
#include "zend_exceptions.h"
#include "ext/spl/spl_exceptions.h"

zend_class_entry *ce_wingdi_exception;
zend_class_entry *ce_wingdi_argexception;
zend_class_entry *ce_wingdi_versionexception;

/* {{{ wingdi_create_error - grabs a message from GetLastError int and throws an exception with it */
void wingdi_create_error(int error, zend_class_entry *ce TSRMLS_DC)
{
	DWORD ret;
	char * buffer = NULL;

	/* Get a system message from the getlasterror value, tell windows to allocate the buffer, and don't
	   sprintf in any args */
	ret = FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER |           
		FORMAT_MESSAGE_IGNORE_INSERTS |            
		FORMAT_MESSAGE_FROM_SYSTEM,                
		NULL,                       
		error,                                     
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&buffer,                           
		0,
		NULL);

	if (ret == 0) {
		zend_throw_exception(ce, "Cannot retrieve error message properly", GetLastError() TSRMLS_CC);
		return;
	}

	zend_throw_exception(ce, buffer, error TSRMLS_CC);
	LocalFree(buffer);
}
/* }}} */

/* {{{ wingdi_juggle_type - takes a zval and juggles the type */
int wingdi_juggle_type(zval *value, int type TSRMLS_DC)
{
	zval tmp_value;

	switch(type)
	{
		case IS_BOOL:
			if (Z_TYPE_P(value) != IS_BOOL) {
				tmp_value = *value;
				zval_copy_ctor(&tmp_value);
				convert_to_boolean(&tmp_value);
				value = &tmp_value;
			}
			return SUCCESS;
		case IS_LONG:
			if (Z_TYPE_P(value) != IS_LONG) {
				tmp_value = *value;
				zval_copy_ctor(&tmp_value);
				convert_to_long(&tmp_value);
				value = &tmp_value;
			}
			return SUCCESS;
		case IS_STRING:
			if (Z_TYPE_P(value) != IS_STRING) {
				tmp_value = *value;
				zval_copy_ctor(&tmp_value);
				convert_to_string(&tmp_value);
				value = &tmp_value;
			}
			return SUCCESS;
	}
	return FAILURE;
}
/* }}} */

/* {{{ unset_abstract_flag - sharing interface definitions amoung objects that implement them */
int unset_abstract_flag(zend_function *func TSRMLS_DC, int num_args, va_list args, zend_hash_key *hash_key)
{
    zend_class_entry *iface_ce = va_arg(args, zend_class_entry *);

    if (func->type == ZEND_INTERNAL_FUNCTION) {
        if (zend_hash_quick_exists(&iface_ce->function_table, hash_key->arKey, hash_key->nKeyLength, hash_key->h)) {
            ((zend_internal_function*)func)->fn_flags &= ~ZEND_ACC_ABSTRACT;
        }
    }

    return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(wingdi_util)
{
	zend_class_entry ce, version_ce;

	INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_NS, "Exception", NULL);
	ce_wingdi_exception = zend_register_internal_class_ex(&ce, zend_exception_get_default(TSRMLS_C), "Exception" TSRMLS_CC);

	INIT_NS_CLASS_ENTRY(ce, PHP_WINGDI_NS, "ArgumentException", NULL);
	ce_wingdi_argexception = zend_register_internal_class_ex(&ce, spl_ce_InvalidArgumentException, "InvalidArgumentException" TSRMLS_CC);

	INIT_NS_CLASS_ENTRY(version_ce, PHP_WINGDI_NS, "VersionException", NULL);
	ce_wingdi_versionexception = zend_register_internal_class_ex(&version_ce, spl_ce_RuntimeException, "RuntimeException" TSRMLS_CC);

	return SUCCESS;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
