<?php
define('ABS_PATH', dirname(__FILE__));
include_once(ABS_PATH . '/config.php');

define('CONVERT_BIN', './hz2py');

if (! ($argc > 2))
{
    echo "Usage: " . __FILE__ . " <src> <pre>\n";
    exit();
}

$src_file = $argv[1];
$pre_file = $argv[2];

$r_fp = fopen($src_file, 'r');
$w_fp = fopen($pre_file, 'w');
if (! $r_fp || ! $w_fp)
{
    echo "Can NOT open file.\n";
    exit(-1);
}

while (! feof($r_fp))
{
    $line = fgets($r_fp, 2048);
    
    $str_len = mb_strlen($line, DEFAULT_ENCODING);
    if ($str_len <= 0 || COMMENT_LINE_FLAG == $line[0])
    {
        continue;
    }

    $exp_data = explode(SEPARATOR, $line);
    if (! (3 == count($exp_data)))
    {
        continue;
    }

    $original = $exp_data[0];
    $original = str_replace('"', '\"', $original);
    $en_len = strlen($original);
    $mb_len = mb_strlen($original, DEFAULT_ENCODING);
    if ($en_len == $mb_len)
    {
        /** the original query is ascii */
        continue; 
    }

    $output = null;
    $spelling = '';         // 全拼
    $simple_short = '';     // 简拼

    $cmd = sprintf("echo \"%s\" | %s -B -d", $original, CONVERT_BIN);
    // echo $cmd; exit();
    $cli_ret = exec($cmd, $output);
    $spelling = $cli_ret;
    $output = null;

    $cmd = sprintf("echo \"%s\" | %s -B -d -f", $original, CONVERT_BIN);
    //echo $cmd; exit();
    $cli_ret = exec($cmd, $output);
    $simple_short = $cli_ret;
    $output = null;

    //echo "{$original} {$spelling} {$simple_short}\n";
    $exp_data[0] = $spelling;
    write_log($w_fp, $exp_data);

    $exp_data[0] = $simple_short;
    write_log($w_fp, $exp_data);
}

fclose($w_fp);
fclose($r_fp);

function write_log(&$fp, &$log_array)
{
    $log = implode("\t", $log_array);
    fwrite($fp, $log, strlen($log));
}
