#1 查看传感器信息
#通过qemu以本地端口映射方式运行编译后的镜像，访问bmcweb页面的网址是：https://localhost:2443/，如果想用IP地址启动请参考：openbmc开发4：启动运行编译镜像。（这里以网桥方式启动，以ip地址方式访问BMC）
#登录后bmcweb后，选择Health，然后点击Sensor，可以看到传感器信息。
#发现并没与风扇信息。 

#2 查看传感器配置文件
#进入meta-live/meta-test/recipes-phosphor/sensors目录，然后看一下目录下的文件,如图所示，发现没有fan传感器的配置文件。

#3 增加fan传感器信息
    #3.1 修改设备树
    #如果已经提取过内核源码，直接修改设备树即可。
    #提取linux内核源码：
    devtool modify linux-aspee
    #该命令会在build/workspace/下生成如下配置文件和源码文件。
    #修改build/sources/linux-aspee/arch/arm/boot/dts/xxx.dts文件：
    #xxx.dts文件是machinie/<machine>.cfg配置文件中“KERNEL_DEVICETREE”字段所指定，修改内核源码中的文件要和该字段指定的dtb相匹配。我这里修改的是aspeed-bmc-opp-test.dts文件，增加fan控制通道、这里打开了4个控制通道。
    
    #3.2 增加配置文件
    #在recipes-phosphor/sensors/phosphor-hwmon/obmc/hwmon/ahb 文件夹下增加一个pwm-tacho-controller@1e786000.conf文件，文件内容如下，这里增加了4个fan传信息。如需增加更多fan信息，需要先设备树中添加。
    #增加配置文件后的sensors下的目录结构： 
    
    #3.3 修改recipes
    #修改recipes-phosphor/sensors/phosphor-hwmon_%.bbappend文件，增加如下内容，
    #以在bmc镜像中增加pwm-tacho-controller@1e786000.conf文件。
    #如果按照“1”添加后编译报错（是由于空格或者table造成），请按照“2”添加，去掉“2”前面的#号即可，效果是一样的。
    
    #3.4 从新编译镜像 
    #为了确保生效先执行清除命令
    bitbake obmc-phosphor-image -c clean
    #然后在执行编译命令
    bitbake obmc-phosphor-image

#4 验证结果
#启动编译后的镜像，登录web，进入sensor页面，发现也没有fan信息，但是有一个“Critical”报错信息。
#点击右上角的“Critical”，可以看到有四个严厉告警信息。由于虚拟机没有风扇，没有获取到风扇转速信息，所以报错。

#5 增加风扇感器过程
    #1、配置文件名
    <path>/recipes-phosphor/sensors/phosphor-hwmon/obmc/hwmon/ahb/apb/pwm-tacho-controller@1e786000.conf
    #1e786000 ——表示aspeed的PWM控制器寄存器地址
    #2、配置文件参数
    #LABEL_fanx——是显示名称
    #PWM_TARGET_fanx——pwm通道几控制fanx
    #WARNLO_fanx——低告警值
    #WARNHI_fanx——高告警值
    #CRITHI_fanx——严重低告警值
    #CRITLO_fanx——严重高告警值
    #后面的告警值是实际值乘以1000，即放大了1000倍，系统在计算时候会自行转换。
    #3、配置文件在镜像中位置
    #配置文件在bmc镜像文件系统中的路径：/etc/default/obmc/hwmon/ahb/apb/ pwm-tacho-controller@1e786000.conf
    #4、fan转速和控制
    #Bmc的rpm采集的对应通道的风扇转速在：/sys/class/hwmon/hwmon0/fanx_input文件中，pwmx输出pwm信号控制风扇转速。
    #改变/sys/class/hwmon/hwmon0/pwmx的值即可配置风扇转速，该值范围是0~255，实际配置的是占空比。
    echo 128 > /sys/class/hwmon/hwmon0/pwm1
    #查看风扇转速
    cat /sys/class/hwmon/hwmon0/fan1_input
    #6、工作方式
    #在内核启动前会先读取dtb设备数中的设备，按照设备树中的参数创建fan设备，内核启动加载驱动后，在文件系统中创建/sys/class/hwmon0/fanx_input文件，这里可以看到一共创建了fan1_input到fan4_input四个文件，这是在设备树中打开的采集通道。最后，启动phosphor-howmon-readd程序，读取/etc/default/obmc/hwmon/ahb/apb/ pwm-tacho-controller@1e786000.conf下的配置文件和/sys/class/hwmon/hwmon0/fanx_input文件，即可在web页面显示fan信息。
    #7 创建风扇传感器步骤
    #1、修改dts，打开对应的fan控制通道。
    #2、在/recipes-phosphor/sensors//phosphor-hwmon/obmc/hwmon/ahb/apb/下按照格式增加新的配置文件pwm-tacho-controller@1e786000.conf。
    #3、修改recipes文件，使编译后的镜像中添加改配置文件。
    #4、重新编译镜像。
