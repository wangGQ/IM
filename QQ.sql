create table user(
account int primary key,
nick_name varchar(17) not null,
password varchar(35) not null,
sex char(1) not null);
 
create table friend
(
id int not null, 
friend_id int not null,
group_name varchar(20) not null,
primary key(id, friend_id),
foreign key(id) references user(account) ON DELETE CASCADE ON UPDATE CASCADE,
foreign key(friend_id) references user(account) ON DELETE CASCADE ON UPDATE CASCADE);

create table user_group (
user_id int not null, 
group_name varchar(20) not null,
primary key(user_id, group_name),
foreign key(user_id) references user(account) ON DELETE CASCADE ON UPDATE CASCADE);

insert into user_group values(1208842581, '我的好友');
insert into user_group values(1208842581, '我的同学');
insert into user_group values(1208842581, '我的家人');
	


insert into friend values(1208842581, 1208842582, '我的好友');
insert into friend values(1208842581, 1208842585, '我的好友');
insert into friend values(1208842581, 1208842608, '我的同学');