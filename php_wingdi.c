/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2012 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Elizabeth Smith <auroraeosrose@gmail.net>                    |
  |         Mark G. Skilbeck   <markskilbeck@php.net>                    |
  +----------------------------------------------------------------------+
*/

#include "php_wingdi.h"

/* ----------------------------------------------------------------
  \Win\Gdi LifeCycle Functions
------------------------------------------------------------------*/

/* Requires winsystem  */
static const zend_module_dep wingdi_deps[] = {
	ZEND_MOD_REQUIRED("winsystem")
	ZEND_MOD_END
};

PHP_MINIT_FUNCTION(wingdi)
{
	PHP_MINIT(wingdi_util)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(wingdi_window)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(wingdi_bitmap)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(wingdi_brush)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(wingdi_color)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(wingdi_devicecontext)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(wingdi_pen)(INIT_FUNC_ARGS_PASSTHRU);
    PHP_MINIT(wingdi_path)(INIT_FUNC_ARGS_PASSTHRU);
	PHP_MINIT(wingdi_region)(INIT_FUNC_ARGS_PASSTHRU);
	return SUCCESS;
}

/* Module entry for wingdi */
zend_module_entry wingdi_module_entry = {
	STANDARD_MODULE_HEADER_EX,
	NULL,
	wingdi_deps,
	"wingdi",
	NULL,
	PHP_MINIT(wingdi),
	NULL,
	NULL,
	NULL,
	NULL,
	PHP_WINGDI_VERSION,
	STANDARD_MODULE_PROPERTIES
};

/* Required for dynamic linking */
#ifdef COMPILE_DL_WINGDI
ZEND_GET_MODULE(wingdi)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */