<?php
use \ZipArchive as ZipArchive;

class HDDenLogger {

  public $logdata;
  private $logfile;

  public function __construct($logfile = false) {

    if (!$logfile){
      $this->logfile = dirname(__FILE__).'/_HDDenLog.txt';
    }

    $this->logfilePrepare(1000000); // мегабайт
  }

  /**
   * Устанавливает путь логфайла
   * @param $path
   *
   * @return bool
   */
  public function setLogfile($path){
    if ($path){
      $this->logfile = $path;
      return true;
    } else {
      return false;
    }
  }

  /**
   * Перезаписывает данные для лога
   * @param $logdata
   */
  public function setLogdata($logdata){
    if (!is_array($logdata)){
      $this->logdata = $logdata . PHP_EOL;
    } else {
      $this->logdata = print_r($logdata, true) . PHP_EOL;
    }
  }

  /**
   * Добавляет данные для лога
   * @param $logdata
   * @param bool $eol
   */
  public function appendLog($logdata, $eol = true){
    if (!is_array($logdata)){
      $this->logdata .= $logdata;
    } else {
      $this->logdata .= print_r($logdata, true);
    }

    if ($eol){
      $this->logdata .= PHP_EOL;
    }
  }

  /**
   * Записывает лог. Может принять строку, если не передано - считывает ранее переданный лог
   * @param string $logdata
   *
   * @return bool
   */
  public function write($logdata = ''){

    // переопределяем предыдущее сохранённое
    if ($logdata){
      $this->setLogdata($logdata);
    } else if ($this->logdata == ''){
      return false; // проверка на пустоту
    }

    // обновляем метку времени
    date_default_timezone_set( 'Europe/Moscow' );
    $date = date('d/m/Y H:i:s', time());

    // сохраняем
    file_put_contents($this->logfile, $date.': '.$this->logdata, FILE_APPEND | LOCK_EX);

    // очищаем
    $this->logdata = '';
  }

  /**
   * Проверяет размер файла лога, запаковывает при превышении переданного лимита
   * @param $maxLogSize
   */
  private function logfilePrepare($maxLogSize){
    if (file_exists($this->logfile)){
      $actualLogSize = filesize($this->logfile);

      // если размер превышен
      if ($actualLogSize >= $maxLogSize){
        $date = date('d-m-Y_H-i-s', time());

        $zipped = false;
        if (class_exists('ZipArchive')){
          $zip_file = dirname($this->logfile).'/_log_'.$date.'.zip';
          $zip = new \ZipArchive;

          if ($zip->open($zip_file, ZIPARCHIVE::CREATE)!==TRUE)
          {
            exit("cannot open <$zip_file>\n");
          }
          $zip->addFile($this->logfile,'log.txt');
          $zip->close();
          $zipped = true;
        }

        // Второй метод сжатия - если не отработал первый.
        $gzipped = false;
        if ( !$zipped ){

          $bkp_to = dirname($this->logfile);
          $bkp_name = '_log_'.$date.'.tar.gz';

          $toarchive = shell_exec('tar -zcvf '.$bkp_to.'/'.$bkp_name.' '.$this->logfile.' ');
          //$toarchive = shell_exec('tar -zcvf file.tar.gz /path/to/filename ');

          $newlogdata = 'Прошли стадию паковки в гз'.PHP_EOL;
          $newlogdata .= var_export($toarchive, true);
          //old_writeLog($newlogdata);

          $gzipped = true;
        }

        if ( $zipped || $gzipped ){
          unlink($this->logfile);
        }
      }
    }

  }
}
