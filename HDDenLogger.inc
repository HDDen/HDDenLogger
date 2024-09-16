<?php
use \ZipArchive as ZipArchive;

class HDDenLogger {

  public $logdata;
  private $logfile;
  private $protect_by_php;

  public function __construct($logfile = false, $protect = true) {

    if (!$logfile){
      if ($protect){
        $this->protect_by_php = true;
        $this->logfile = dirname(__FILE__).'/_HDDenLog.txt.php';
      } else {
        $this->protect_by_php = false;
        $this->logfile = dirname(__FILE__).'/_HDDenLog.txt';
      }
      
    } else {
      $logfile_folder = rtrim(dirname($logfile), '\/');
      if ($logfile_folder !== ''){
        if ( !file_exists( $logfile_folder ) || !is_dir( $logfile_folder) ) {
          mkdir($logfile_folder);
        }
      }

      $this->logfile = $logfile;
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

    // проверяем путь
    $log_folder = dirname($this->logfile);
    if ( !file_exists( $log_folder ) && !is_dir( $log_folder ) ) {
      mkdir( $log_folder, 0777, true );
    }

    // сохраняем
    file_put_contents($this->logfile, $date.': '.$this->logdata, FILE_APPEND | LOCK_EX);

    // очищаем
    $this->logdata = '';
  }

  /**
   * Проверяет размер файла лога, запаковывает при превышении переданного лимита, защищает если новый
   * @param $maxLogSize
   */
  private function logfilePrepare($maxLogSize){
    clearstatcache();
    if (file_exists($this->logfile)){
      $actualLogSize = filesize($this->logfile);

      // если размер превышен
      if ($actualLogSize >= $maxLogSize){
        $date = date('Y-m-d_H-i-s', time());

        $zipped = false;
        if (class_exists('ZipArchive')){
          $zip_file = dirname($this->logfile).'/_log-'.$_SERVER['HTTP_HOST'].'_'.$date.'.zip';
          $zip = new \ZipArchive;

          if ($zip->open($zip_file, ZIPARCHIVE::CREATE)!==TRUE)
          {
            exit("cannot open <$zip_file>\n");
          }
          $zip->addFile($this->logfile,'log-'.$_SERVER['HTTP_HOST'].'_'.$date.'.txt');
          $zip->close();
          $zipped = true;
        }

        // Второй метод сжатия - если не отработал первый.
        $gzipped = false;
        if ( !$zipped ){

          $bkp_to = dirname($this->logfile);
          $bkp_name = '_log-'.$_SERVER['HTTP_HOST'].'_'.$date.'.tar.gz';

          $toarchive = shell_exec('tar -zcvf '.$bkp_to.'/'.$bkp_name.' '.$this->logfile.' ');
          //$toarchive = shell_exec('tar -zcvf file.tar.gz /path/to/filename ');

          $newlogdata = 'Прошли стадию паковки в гз'.PHP_EOL;
          $newlogdata .= var_export($toarchive, true);
          //old_writeLog($newlogdata);

          $gzipped = true;
        }

        if ( $zipped || $gzipped ){
          unlink($this->logfile);
          file_put_contents($this->logfile, '<?php die(); /*'.PHP_EOL, LOCK_EX);
        }
      }
    } elseif ($this->protect_by_php) {
      file_put_contents($this->logfile, '<?php die(); /*'.PHP_EOL, LOCK_EX);
    }

  }
}
