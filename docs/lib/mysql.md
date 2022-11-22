### MySQL

Melang only support MySQL 8.0, because that version supports asynchronous functions.

If you want to activate MySQL APIs, you should install MySQL library and header files before Melang installation.



### Import

```
mysql = Import('mysql');
```



Here is the Set *Mysql*:

```
Mysql {
  fd;
  connect(host, port, dbname, username, password);
  close();
  autocommit(mode);
  commit();
  rollback();
  error();
  errno();
  execute(sql);
};
```

##### fd

fd is a memory block of MySQL structure in C. So it's value is a memory address, and it's read-only property.



##### connect

Create a session on MySQL server.

```
connect(host, port, dbname, username, password);
```

Input:

- host - a host string of MySQL server.
- port - an integer port number that MySQL server listening.
- dbname - string database name.
- username - string username.
- password - string password.

Return value:

- *true* on success, otherwise *false*.

Error:

- any error will be thrown if encountered.



##### close

Close session.

```
close();
```

Input: None.

Return value:

- always *nil*.

Error:

- any error will be thrown if encountered.



##### autocommit

Switch autocommit mode.

```
autocommit(mode);
```

Input:

- mode - a boolean value. Set *true* If want to auto commit, otherwise *false*.

Return value:

- *true* on success, otherwise *false*.

Error:

- any error will be thrown if encountered.



##### commit

Commit the current transaction.

```
commit();
```

Input: None.

Return value:

- *true* on success, otherwise *false*.

Error:

- any error will be thrown if encountered.



##### rollback

Roll back the current transaction.

```
rollback();
```

Input: None.

Return value:

- *true* on success, otherwise *false*.

Error:

- any error will be thrown if encountered.



##### error

Get error message.

```
error();
```

Input: None.

Return value:

- a string error message returned on success, otherwise *false* returned.

Error:

- any error will be thrown if encountered.



##### errno

Get error number.

```
errno();
```

Input: None.

Return value:

- an integer error number returned on success, otherwise *false* returned.

Error:

- any error will be thrown if encountered.



##### execute

Execute SQL.

```
execute(sql);
```

Input:

- sql - a string SQL.

Return value:

- Reading operation returns an array whose every element is also an array that is equivalent to a record in query results.
- Writing operation returns a boolean value. *true* on success, otherwise *false*.

Error:

- any error will be thrown if encountered.



##### Example

There is a database named *test*, and there is a table *people* in it:

```sql
CREATE TABLE `people` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `age` tinyint unsigned NOT NULL DEFAULT '0',
  `name` varchar(32) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
```

Our program:

```
mysql = Import('mysql');
sys = Import('sys');

m = $mysql; // or m = $Mysql; both are the same. the value of mysql is 'Mysql'.
m.connect('127.0.0.1', 3306, 'test', 'root', '.../*password*/');
sys.print(m.execute('insert into `people` (name, age) values("Tom", 18)'));
result = m.execute('select `name`, `age` from `people`');
sys.print(result);
m.close();
```

The output is:

```
true
[[Tom, 18, ], ]
```

