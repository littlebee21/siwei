#!/bin/bash
JDK_VERSION="java-1.8.0-openjdk-devel.x86_64"
FLAG_CHAR="y"
WELCOME_TITLE="### 快捷安装jdk mysql nginx 脚本 ###"
ECHO_FILE="README_ME.txt"
echo $WELCOME_TITLE
# 
echo "更新yum--请手动执行 yum -y update ，之后重新打开。"
#yum -y update
# openjdk安装
echo "## 是否安装jdk?(y or n) ##"
read CASE
if [ "$CASE" == "$FLAG_CHAR" ]
then
    yum install $JDK_VERSION
    echo "export JAVA_HOME=/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.171-8.b10.el6_9.x86_64" >> /etc/profile
    echo "export CLASSPATH=.:$JAVA_HOME/jre/lib/rt.jar:$JAVA_HOME/lib/dt.jar:$JAVA_HOME/lib/tools.jar" >> /etc/profile
    echo "export PATH=$PATH:$JAVA_HOME/bin" >> /etc/profile
    source  /etc/profile
    java -version
    echo "## jdk安装完成 ##"
fi

# nginx 安装
echo "## 是否安装nginx?(y or n) ##"
read CASE
if [ "$CASE" == "$FLAG_CHAR" ]
then
    yum install -y epel-release
    yum -y update
    yum -y install nginx
    systemctl enable nginx
    nginx -v
    echo "### nginx ###" >> $ECHO_FILE
    echo "配置文件路径  /etc/nginx/conf.d" >> $ECHO_FILE
    cat > /etc/nginx/conf.d/default.conf.tmp << \EOF
    server {
    listen       443 ssl;
    server_name  wxsxw.xx.club;
    client_max_body_size    1000m;
    gzip  on;
    gzip_min_length  1k;
    gzip_buffers     4 16k;
    gzip_http_version 1.1;
    gzip_comp_level 9;
    gzip_types       text/plain application/x-javascript text/css application/xml text/javascript application/x-httpd-php application/javascript application/json;
    gzip_disable "MSIE [1-6]\.";
    gzip_vary on;
    ssl_certificate      /etc/nginx/conf.d/ssl/wxsxw.xx.club.crt;
    ssl_certificate_key  /etc/nginx/conf.d/ssl/wxsxw.xx.club.key;

    ssl_session_cache    shared:SSL:1m;
    ssl_session_timeout  5m;

    ssl_ciphers  HIGH:!aNULL:!MD5;
    ssl_prefer_server_ciphers  on;

    location / {
         proxy_pass http://localhost:8082;
    }

    location /wx721 {
         root /etc/nginx/html;
	 index index.html;
    }

}

server {
    listen       80;
    server_name  wxsxw.xx.club;
    client_max_body_size    1000m;
    gzip  on;
    gzip_min_length  1k;
    gzip_buffers     4 16k;
    gzip_http_version 1.1;
    gzip_comp_level 9;
    gzip_types       text/plain application/x-javascript text/css application/xml text/javascript application/x-httpd-php application/javascript application/json;
    gzip_disable "MSIE [1-6]\.";
    gzip_vary on;

    return      301 https://$server_name$request_uri;
}
EOF
    echo "## nginx安装完成 ##"
fi

# mysql5.7
echo "## 是否安装mysql?(y or n) ##"
read CASE
if [ "$CASE" == "$FLAG_CHAR" ]
then
    cat > /etc/yum.repos.d/mysql-community.repo << EOF
[mysql-connectors-community]
name=MySQL Connectors Community
baseurl=https://mirrors.tuna.tsinghua.edu.cn/mysql/yum/mysql-connectors-community-el7-\$basearch/
enabled=1
gpgcheck=1
gpgkey=https://repo.mysql.com/RPM-GPG-KEY-mysql

[mysql-tools-community]
name=MySQL Tools Community
baseurl=https://mirrors.tuna.tsinghua.edu.cn/mysql/yum/mysql-tools-community-el7-\$basearch/
enabled=1
gpgcheck=1
gpgkey=https://repo.mysql.com/RPM-GPG-KEY-mysql

[mysql-5.6-community]
name=MySQL 5.6 Community Server
baseurl=https://mirrors.tuna.tsinghua.edu.cn/mysql/yum/mysql-5.6-community-el7-\$basearch/
enabled=0
gpgcheck=1
gpgkey=https://repo.mysql.com/RPM-GPG-KEY-mysql

[mysql-5.7-community]
name=MySQL 5.7 Community Server
baseurl=https://mirrors.tuna.tsinghua.edu.cn/mysql/yum/mysql-5.7-community-el7-\$basearch/
enabled=1
gpgcheck=1
gpgkey=https://repo.mysql.com/RPM-GPG-KEY-mysql

[mysql-8.0-community]
name=MySQL 8.0 Community Server
baseurl=https://mirrors.tuna.tsinghua.edu.cn/mysql/yum/mysql-8.0-community-el7-\$basearch/
enabled=1
gpgcheck=1
gpgkey=https://repo.mysql.com/RPM-GPG-KEY-mysql
EOF

    wget http://dev.mysql.com/get/mysql57-community-release-el7-8.noarch.rpm
    yum localinstall mysql57-community-release-el7-8.noarch.rpm
    yum clean all
    yum makecache
    yum install mysql-community-server
    systemctl start mysqld
    systemctl status mysqld
    systemctl enable mysqld
    systemctl daemon-reload
    default_pass=$(grep 'temporary password' /var/log/mysqld.log)
    echo "### mysql ###" >> $ECHO_FILE
    echo "mysql默认密码 $default_pass"
    echo "mysql默认密码 "$default_pass >> $ECHO_FILE
    cat > sql.readme.txt << EOF
    #mysql 8.0
#修改root密码
ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY 'zw1%l73#qLD6hMpi';
#创建远程用户
create user hlsd@'%' identified  by 'zw1%l73#qLD6hMpi';
grant all privileges on *.* to hlsd@'%' with grant option;
flush privileges;
ALTER USER 'hlsd'@'%' IDENTIFIED WITH mysql_native_password BY 'zw1%l73#qLD6hMpi';
flush privileges;

#mysql5.7
GRANT ALL PRIVILEGES ON *.* TO 'hlsd'@'%'IDENTIFIED BY 'zw1%l73#qLD6hMpi' WITH GRANT OPTION;
flush privileges;
EOF
    echo "## mysql安装完成 ##"
fi


 
# 防火墙开启
echo "## 是否开启防火墙?(y or n) ##"
read CASE
if [ "$CASE" == "$FLAG_CHAR" ]
then
    systemctl start firewalld.service
    firewall-cmd --permanent --add-port={80,3306,443,81-88}/tcp    
    firewall-cmd --reload
    firewall-cmd --list-all    
    echo "### 防火墙修改完成  ###"
    echo "已开端口 80,3306,443,81-88,8082-8088" >> $ECHO_FILE
fi 