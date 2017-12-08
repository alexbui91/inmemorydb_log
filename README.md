--------------------
How to build and run this code
--------------------
- Prepare to build:
  + gcc 5.4+
  + make sure your machine supports g++ 11
  # install boost
  sudo apt-get install libboost-all-dev
  # install libpng libjpeg
  sudo apt-get install libpng-dev
  # prepare python package: 
  $ pip install python-resize-image 
  # because this data set is quite large, we have to resize all images inside to the smaller size to test the execution easily.
- Build server:
  $ unzip assignment5.zip
  # firstly, copy dataset to source folder
  $ cp -r KITTI_dataset assignment5
  $ mkdir dataset
  $ mkdir gray
  $ python resize.py
  $ cd Debug
  $ mkdir snapshot
  $ make clean
  $ make all
- Run server (maybe use run file contained):
  $ ./assignment5 port_no checkpoint_interval ../dataset ../gray debug_mode recover_mode random_shutdow_server 
  # port_no: 8888 (default)
  # checkpoint_interval: (float type) 0.5s => 0.5
  # debug_mode, recover_mode: boolean 0 or 1
  # random_shutdow_server: auto shutdow server to test uncommitted transactions

