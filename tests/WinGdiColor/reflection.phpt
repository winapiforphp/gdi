--TEST--
Reflection for \Win\Gdi\Color Class
--SKIPIF--
<?php
	if (!extension_loaded('wingdi')) {
		die("skip wingdi extension not available\n");
	}
?>
--FILE--
<?php
Reflection::export(new ReflectionClass('\Win\Gdi\Color'));
?>
--EXPECTF--
Class [ <internal:wingdi> class Win\Gdi\Color ] {

  - Constants [0] {
  }

  - Static properties [0] {
  }

  - Static methods [0] {
  }

  - Properties [4] {
    Property [ <default> public $red ]
    Property [ <default> public $green ]
    Property [ <default> public $blue ]
    Property [ <default> public $hex ]
  }

  - Methods [2] {
    Method [ <internal:wingdi, ctor> public method __construct ] {

      - Parameters [3] {
        Parameter #0 [ <optional> $red ]
        Parameter #1 [ <optional> $green ]
        Parameter #2 [ <optional> $blue ]
      }
    }

    Method [ <internal:wingdi> public method __toString ] {
    }
  }
}