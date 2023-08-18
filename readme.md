# 项目名称

## 简介

该项目用于GD32F30x系列单片机。

## 文件结构

- Doc/            # 存放项目文档
- Examples/       # 存放示例代码
- Project/        # 项目文件夹
  - Application/    # 存放工程文件
    - Firmware/       # 存放启动文件和CMSIS相关文件
    - User/           # 存放项目代码
      - hardware/       # 存放硬件驱动代码
      - system/         # 存放系统组件代码
  - Bootloader/     # 存放bootloader文件 
  - Settings/       # 存放工程配置文件 用于快速配置工程
- Tools           # 用于存放相关工具




## 使用说明


1. 在 `GD32F303/Project/Application/Firmware/CMSIS/GD/GD32F30x/Source/ARM` 目录中为启动文件，根据特定硬件平台进行配置。
2. 在 `GD32F303/Project/Firmware/CMSIS` 目录中使用相应的CMSIS库和头文件，提供硬件抽象层的支持。
3. 可以在 `GD32F303/Examples` 目录中查找示例代码，并根据需要进行参考或调试。
4. 使用 `GD32F303/Project/User` 目录中存放功能函数、驱动代码等。
5. 在 `GD32F303\Doc` 目录中存放项目相关的文档，包括项目说明、使用手册等。

## 注意事项

- 该说明文档提供了项目的文件结构以及一些使用说明。请根据实际情况和项目需求进行适当的调整和补充


<!-- /*!
    \file    readme.txt
    \brief   description of led spark with systick, USART print and key example

    \version 2017-02-10, V1.0.0, firmware for GD32F30x
    \version 2018-10-10, V1.1.0, firmware for GD32F30x
    \version 2018-12-25, V2.0.0, firmware for GD32F30x
    \version 2020-09-30, V2.1.0, firmware for GD32F30x 
*/

/*
    Copyright (c) 2020, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

  This example is based on the GD32307C-EVAL-V1.1 board, it provides a
description of SysTick configuration, use of EVAL_COM and key. 
  
  -->