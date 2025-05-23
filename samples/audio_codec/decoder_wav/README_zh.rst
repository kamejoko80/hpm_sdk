.. _audio_wav_decoder:

Audio Wav Decoder
==================================

概述
------

音频编解码示例工程展示了如何播放WAV格式音频。
在这个工程中，读取SD卡中的wav格式文件然后进行播放
MCU会通过I2S接口将解码的数据发送给音频编解码器，音频编解码器会解码收到的数据并播放。

工程配置
------------

- 在文件`CMakeLists.txt`中, 根据需求设置音频播放器件的类型，默认使用DAO播放。如存在板载音频编解码芯片， 可以设置匹配的音频编解码芯片类型， 例如："set(CONFIG_CODEC "sgtl5000")"。

硬件设置
------------

- 根据工程配置， 使用DAO播放时将喇叭接到 :ref:`DAO <board_resource>` 接口， 使用音频编解码芯片播放时将耳机接到音频编解码芯片的 :ref:`耳机 <board_resource>` 接口。

已知问题
------------

使用DAO播放某些音频时会产生噪声。

运行现象
------------

当工程正确运行后，选择音乐名称，然后耳机会播放这首音乐，而串口终端会输出如下信息：

.. code-block:: console

   audio codec example
   SD card has been mounted successfully


   ***********Music List**********

   **Enter any non-numeric key to change pages or music number to choose music**


   ***********Page**********
   0: music1.wav

   1: music2.wav

   2: music3.wav

   3: music4.wav

   4: music5.wav

   5: music6.wav

   6: mucsic7.wav

   7: Ring10.wav

   8: on_phone.wav

   9: calling.wav

