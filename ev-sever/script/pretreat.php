<?php
define('ABS_PATH', dirname(__FILE__));
include_once(ABS_PATH . '/config.php');

define('CONVERT_BIN', './hz2py');
define('MULTI_TONE',  '|');
define('MULTI_SEPARATOR', ' ');

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

    $cmd = sprintf("echo \"%s\" | %s ", $original, CONVERT_BIN);
    // echo $cmd; exit();
    $cli_ret = exec($cmd, $output);
    $spelling = $cli_ret;
    $output = null;

    $cmd = sprintf("echo \"%s\" | %s -f", $original, CONVERT_BIN);
    //echo $cmd; exit();
    $cli_ret = exec($cmd, $output);
    $simple_short = $cli_ret;
    $output = null;

    //echo "{$original} {$spelling} {$simple_short}\n";
    $exp_data[0] = get_multiple_tone($spelling);
    write_log($w_fp, $exp_data);

    $exp_data[0] = get_multiple_tone($simple_short);
    write_log($w_fp, $exp_data);
}

fclose($w_fp);
fclose($r_fp);

function write_log(&$fp, &$log_array)
{
    $log = implode("\t", $log_array);
    fwrite($fp, $log, strlen($log));
}

function get_multiple_tone($str)
{
    $sub_exp = explode(MULTI_SEPARATOR, $str);
    foreach ($sub_exp as $key => $value)
    {
        if (false !== strpos($value, MULTI_TONE))
        {
            $multi_exp = explode(MULTI_TONE, $value);
            $sub_exp[$key] = $multi_exp[0];
        }
    }

    $str = implode('', $sub_exp);
    return $str;
}
