--TEST--
\Win\Gdi\Color generic properties
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

$class->foobar = 255;
echo "$class->foobar\n";

$class->{1} = 255;
echo "{$class->{1}}\n";

var_dump($class);

unset($class->foobar);
unset($class->{1});

var_dump($class);
?>
--EXPECTF--
#000000
255
255
object(Win\Gdi\Color)#%d (6) {
  ["red"]=>
  int(0)
  ["green"]=>
  int(0)
  ["blue"]=>
  int(0)
  ["hex"]=>
  int(0)
  ["foobar"]=>
  int(255)
  ["1"]=>
  int(255)
}
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