
System requirements and dependencies

* Operating system: Linux distribution. Ubuntu 12.0 o higher recommended 
* RAM: 900 MB
* Processor: 1.73 Ghz
* ffmpeg libraries

/****  EXECUTION ****/
		
In order to launch the program, please write the following via console: 
	$./fds [video_name.avi] [file_name.csv]  
	Where:
	[video_name.avi] is the name of the video file under analysis. Videos should be provided in AVI format. 
	Codecs  ffmpeg  should be present in the system.
	[file_name.csv] is the name of the file where the results will be stored.
	In the file_name is not provided, the program will ask for it. If no answer is given, the program will use
	[Data_n] as default name (n within 1 to 29). 
