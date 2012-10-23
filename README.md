mysql_extend
============

"MySQL-Monitoring-Proxy-Tool" for Zabbix

This tool can be utilized to gather behavioral measurement data or configuration data of MySQL servers in a efficient way. 
It can be used as an external check for the zabbix monitoring system.
The benefit for using this tool is a very low overhead for gathering the measurements.

MySQL Measurements and Configuration details:
 * "SHOW GLOBAL VARIABLES"
 * "SHOW GLOBAL STATUS"
 * "SHOW SLAVE STATUS"

What it does in general:
 * On invocation the tool creates a lock which blocks all other running instances of this tool
 * It searches for a shared memory segment which is used for caching measurement values
   * If the segment does not exist a new segment is created by the tool
   * A lookup is performed to find out if there are already cached values for the specified host
   * If the stored measures are not older than 60 seconds the specified measure is provided to stdout and the program flow is terminated
 * If the store values are older than 60 seconds or not stored values are available an database connection is established and the measurements listed above are executed
   * The results are stored in the shared memory segment (if there are any) with the timestamp of the fetch
   * If no results are available the old results remain in the shared memory segment
 * End of program flow, release the lock to allow other instances of the tool to continue their work

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

Call the tool with parameters like this:
```
./mysql_extend -P 3306 -t 2 Binlog_cache_use foo.bar.de
./mysql_extend --help
```

The directory "zabbix" contains a example zabbix monitoring template which measures and notifies.
The template was created for zabbix release 1.8.

Installation:
 * Install "mysql_extend" on the zabbix-server or zabbix-proxy (make install)
 * Import the zabbix-template to your zabbix-server
 * Assign hosts to the template and overload the macros of the template at host level.

Review file "zabbix/Custom_-_Service_-_MySQL.html" to get detailed information about the behavior of this template.
(see also: http://htmlpreview.github.com/?https://github.com/digitalmediacenter/mysql_extend/master/zabbix/Custom_-_Service_-_MySQL.html)

Licence and Authors
-------------------

Additional authors are very welcome - just submit you patches as pull requests.

 * Andreas Heil <andreas.heil@dmc.de>
 * Marc Schoechlin <marc.schoechlin@dmc.de>

License - see: LICENSE.txt