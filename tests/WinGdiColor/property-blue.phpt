--TEST--
\Win\Gdi\Color->$blue property
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
echo "$class\n";

$class->blue = 255;
echo "$class\n";

$class->blue = 654;
echo "$class\n";

$class->blue = -5;
echo "$class\n";

$class->hex = 'FFF';
echo "$class->blue\n";

unset($class->blue);

var_dump($class);
?>
--EXPECTF--
#000000
#00ff00
#00ff00
#000000
255

Warning: main(): Property blue cannot be unset in %s on line %d
object(Win\Gdi\Color)#%d (4) {
  ["red"]=>
  int(255)
  ["green"]=>
  int(255)
  ["blue"]=>
  int(255)
  ["hex"]=>
  int(16777215)
}