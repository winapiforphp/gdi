--TEST--
\Win\Gdi\Color->$hex property
--SKIPIF--
<?php
	if (!extension_loaded('wingdi')) {
		die("skip wingdi extension not available\n");
	}
?>
--FILE--
<?php
use \Win\Gdi\Color;

$class = new Color();
echo "$class->hex\n";

$class->hex = 555555;
echo "$class->hex\n";

$class->hex = 'fff';
echo "$class->hex\n";

$class->hex = '#f0f';
echo "$class->hex\n";

$class->hex = '#cc0000';
echo "$class->hex\n";

$class->hex = 'cccccc';
echo "$class->hex\n";

$class->hex = '654654353';
echo "$class->hex\n";

$class->hex = 'b7b7b7';
echo "$class->hex\n";

$class->hex = '-1-1-1';
echo "$class->hex\n";

unset($class->hex);

var_dump($class);
?>
--EXPECTF--
0
555555
16777215
16711935
204
13421772
342929
12040119
0

Warning: main(): Property hex cannot be unset in %s on line %d
object(Win\Gdi\Color)#%d (4) {
  ["red"]=>
  int(0)
  ["green"]=>
  int(0)
  ["blue"]=>
  int(0)
  ["hex"]=>
  int(0)
}