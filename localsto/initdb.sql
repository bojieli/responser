CREATE TABLE IF NOT EXISTS student ( -- 学号到学生姓名的映射，只读，从云端下载
	student_id TEXT UNIQUE,
	name TEXT
);
CREATE TABLE IF NOT EXISTS product ( -- 硬件ID到学号的映射，可读写，永久保存并上传
	product_id INTEGER UNIQUE,
	student_id INTEGER
);
CREATE TABLE IF NOT EXISTS lecture ( -- 课堂号，insert only，永久保存并增量上传
	id INTEGER PRIMARY KEY, -- 自动递增（永久保存，以免从1重新开始）
	begin_time INTEGER,     -- 连接数据库服务器的时间戳
	end_time INTEGER,       -- 上课结束的时间戳
	synced INTEGER          -- 已经同步到云端的为1
);
CREATE TABLE IF NOT EXISTS register ( -- 课堂签到注册，insert only，上传后删除
	product INTEGER,        -- 硬件ID
	lecture INTEGER,        -- lecture表的ID
	reg_time INTEGER        -- 开始上课之前的记为0，不然记为开始上课后的秒数
);
CREATE TABLE IF NOT EXISTS problem ( -- 每道题的信息，insert only，上传后删除
	lecture INTEGER,    -- lecture表的id
	problem INTEGER,    -- 这堂课的第几道题
	begin_time INTEGER, -- 答题开始时间，时间戳
	end_time INTEGER,   -- 答题结束时间，时间戳
	correct_ans INTEGER -- 正确答案，1字节
);
CREATE TABLE IF NOT EXISTS answer ( -- 学生的答案，insert only，上传后删除
	lecture INTEGER, -- lecture表的id
	problem INTEGER, -- 这堂课的第几道题
	product INTEGER, -- 硬件ID
	ans INTEGER,     -- 学生输入的答案，1字节
	ans_time INTEGER -- 答题时间，从答题开始算起的秒数
);