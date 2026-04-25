-- =============================================
-- 1. game_account 库：账号、登录、token、封禁
-- =============================================
CREATE DATABASE IF NOT EXISTS game_account DEFAULT CHARSET utf8mb4;
USE game_account;

DROP TABLE IF EXISTS account;
CREATE TABLE account (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    account VARCHAR(64) NOT NULL UNIQUE,        -- 账号
    password VARCHAR(64) NOT NULL,              -- 密码（md5）
    token VARCHAR(128) DEFAULT '',             -- 登录凭证
    uid BIGINT DEFAULT 0,                       -- 绑定角色UID
    status TINYINT DEFAULT 0,                   -- 0正常 1封禁
    create_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    login_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_account(account),
    INDEX idx_uid(uid)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- =============================================
-- 2. game_user 库：玩家角色数据（分32张表，高性能）
-- =============================================
CREATE DATABASE IF NOT EXISTS game_user DEFAULT CHARSET utf8mb4;
USE game_user;

-- 自动创建 32 张分表：user_role_00 ~ user_role_31
DROP PROCEDURE IF EXISTS create_user_role_tables;
DELIMITER //
CREATE PROCEDURE create_user_role_tables()
BEGIN
    DECLARE i INT DEFAULT 0;
    WHILE i < 32 DO
        SET @table_name = CONCAT('user_role_', LPAD(i, 2, '0'));
        SET @sql = CONCAT('
        CREATE TABLE IF NOT EXISTS ', @table_name, ' (
            uid BIGINT PRIMARY KEY,
            name VARCHAR(32) NOT NULL,
            level INT DEFAULT 1,
            exp BIGINT DEFAULT 0,
            gold BIGINT DEFAULT 1000,
            diamond INT DEFAULT 0,
            vip INT DEFAULT 0,
            status TINYINT DEFAULT 0,
            create_time DATETIME DEFAULT CURRENT_TIMESTAMP,
            update_time DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
        ');
        PREPARE stmt FROM @sql;
        EXECUTE stmt;
        DEALLOCATE PREPARE stmt;
        SET i = i + 1;
    END WHILE;
END //
DELIMITER ;
CALL create_user_role_tables();
DROP PROCEDURE IF EXISTS create_user_role_tables;

-- 玩家道具背包（分表）
DROP PROCEDURE IF EXISTS create_user_item_tables;
DELIMITER //
CREATE PROCEDURE create_user_item_tables()
BEGIN
    DECLARE i INT DEFAULT 0;
    WHILE i < 32 DO
        SET @table_name = CONCAT('user_item_', LPAD(i, 2, '0'));
        SET @sql = CONCAT('
        CREATE TABLE IF NOT EXISTS ', @table_name, ' (
            uid BIGINT,
            item_id INT,
            count INT DEFAULT 0,
            update_time DATETIME DEFAULT CURRENT_TIMESTAMP,
            PRIMARY KEY(uid, item_id)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
        ');
        PREPARE stmt FROM @sql;
        EXECUTE stmt;
        DEALLOCATE PREPARE stmt;
        SET i = i + 1;
    END WHILE;
END //
DELIMITER ;
CALL create_user_item_tables();
DROP PROCEDURE IF EXISTS create_user_item_tables;

-- =============================================
-- 3. game_global 库：全局配置、活动、公告、服数据
-- =============================================
CREATE DATABASE IF NOT EXISTS `game_global` DEFAULT CHARSET utf8mb4;
USE `game_global`;

CREATE TABLE IF NOT EXISTS global_config (
    id INT PRIMARY KEY AUTO_INCREMENT,
    `key` VARCHAR(64) UNIQUE NOT NULL,
    `value` TEXT,
    `desc` VARCHAR(255),
    update_time DATETIME DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS global_activity (
    id INT PRIMARY KEY AUTO_INCREMENT,
    act_id INT UNIQUE NOT NULL,
    title VARCHAR(128),
    content TEXT,
    start_time DATETIME,
    end_time DATETIME,
    status TINYINT DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS global_announce (
    id INT PRIMARY KEY AUTO_INCREMENT,
    content TEXT,
    sort INT DEFAULT 0,
    create_time DATETIME DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- =============================================
-- 4. game_log 库：全量日志（防外挂、审计、排查）
-- =============================================
CREATE DATABASE IF NOT EXISTS game_log DEFAULT CHARSET utf8mb4;
USE game_log;

CREATE TABLE IF NOT EXISTS log_login (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    uid BIGINT,
    ip VARCHAR(64),
    device VARCHAR(255),
    login_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_uid(uid),
    INDEX idx_time(login_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS log_coin (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    uid BIGINT,
    reason VARCHAR(64),
    gold BIGINT,
    diamond INT,
    create_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_uid(uid),
    INDEX idx_time(create_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS log_operate (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    uid BIGINT,
    op VARCHAR(64),
    params TEXT,
    create_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_uid(uid),
    INDEX idx_time(create_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- =============================================
-- 5. game_pay 库：充值、订单、发货、退款
-- =============================================
CREATE DATABASE IF NOT EXISTS game_pay DEFAULT CHARSET utf8mb4;
USE game_pay;

CREATE TABLE IF NOT EXISTS pay_order (
    order_id VARCHAR(128) PRIMARY KEY,
    uid BIGINT NOT NULL,
    product_id VARCHAR(64),
    amount INT NOT NULL,               -- 分
    diamond INT NOT NULL,              -- 发放钻石
    status TINYINT DEFAULT 0,          -- 0待支付 1已完成 2退款
    create_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    pay_time DATETIME DEFAULT NULL,
    INDEX idx_uid(uid),
    INDEX idx_status(status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS pay_delivery (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    order_id VARCHAR(128),
    uid BIGINT,
    diamond INT,
    deliver_time DATETIME DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;