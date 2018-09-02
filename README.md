# ncp
Network copy -- Copy files over the network

## Usage
This software can work in two modes:
  * **Client**: You choose the file to send and the address to be sent.

  Usage: **./ncp --addr=IP [--file=FILE] [--port=PORT]**    

| Option     | Description                        | Type                                                                  |
|:----------:|:----------------------------------:|:---------------------------------------------------------------------:|
| --addr     | Specify IP address to connect to.  | Mandatory.                                                            |
| --file     | Specify file to be sent.           | Optional. If no file is specified, ncp will read from standart input. |
| --port     | Specify port to connect.           | Optional. If no port is specified, ncp will use the default one.      |


  * **Server**: You listen to a client which will send the file.

  Usage: **./ncp --listen [--out=FILE] [--port=PORT]**

| Option     | Description                                                      | Type                                                                  |
|:----------:|:----------------------------------------------------------------:|:---------------------------------------------------------------------:|
| --listen   | Set server mode.                                                 | Mandatory.                                                            |
| --out      | Specify file name that will be used to save the received file.   | Optional. If no file is specified, ncp will write to standart output. |
| --port     | Specify port to connect.                                         | Optional. If no port is specified, ncp will use the default one.      |

  * _This options work on both modes:_

| Option     | Description            | Type       |
|:----------:|:----------------------:|:----------:|
| --no-bar   | Disable progress bar.  | Optional.  |


## Features

* **Real time progress bar**: While data is on the go, ncp will show a progress bar to help the user understand how file copy is going.

* **Speed and time statistics**: At the end of the transfer, ncp will show time and average speed of the transaction.

* **Hash compare**: At the same time the transfer is taking place, it's also calculating _sha1_ hash of the file being sent(by client) and the hash of the file being received(by the server). At the end, client will send it to the server which will compare both to determine if file was transferred with no errors.
