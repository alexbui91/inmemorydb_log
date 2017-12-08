--------------------
How to build and run this code
--------------------
- Prepare to build:
  + gcc 5.4+
  + make sure your machine supports g++ 11  
- Build: 
  $ unzip assignment5
  $ cd ClientLog/Debug
  $ make clean
  $ make all
- Run (maybe use run file contained):
  $ ./ClientDB port_no auto_mode host_addr interval
  # port_no - ex. 8888
  # auto_mode - 0 or 1. switch between autonomously connect server with demo image or manually type command 
  # command list: 
  	# ls (list all list images)
  	# convert image_name.png
  # host_addr - "127.0.0.1"
  # interval - if mode is auto, interval time is set to auto connnect 
  
  
