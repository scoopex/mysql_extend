mysql_extend
============

"MySQL-Monitoring-Proxy" for Zabbix

Compile and Install
-------------------
Prerequisites:
 * C-compiler
 * GNU Make
 * MySQL-Client libraries (dev)

This will install the binary 'mysql_extend' to /etc/zabbix/externalscripts:
```
git clone git://github.com/digitalmediacenter/mysql_extend.git mysql_extend
cd mysql_extend
make
make install
```

Configure
---------
create a .my.cnf in the zabbix-user's home:
```
[mysql_extend]
user = zabbix
password = somegoodpassword
```

Usage
-----
Configure items like this in zabbix:
```
Description........: MySQL Com_alter_function
Type...............: External check
Key................: mysql_extend[Com_alter_function -P3306]
Type of information: Numeric (unsigned)
Data type..........: Decimal
Update interval....: 300
Store value........: Delta (simple change)
```

