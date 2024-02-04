DB.CreateTables = function()
	DB.SimpleExecute([[
		CREATE TABLE IF NOT EXISTS `accounts` (
			`id` INTEGER NOT NULL PRIMARY KEY,
			`name` VARCHAR(32) NOT NULL,
			`password` INTEGER NOT NULL,
			`level` INTEGER DEFAULT 1,
			`exp` INTEGER DEFAULT 0,
			`money` INTEGER DEFAULT 0,
			`ruby` INTEGER DEFAULT 0,
			`clan_id` INTEGER DEFAULT 0,
			`upgrade_points` INTEGER DEFAULT 0,
			`create_date` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
			`name_registred` VARCHAR(16) NOT NULL,
			`name_latest` VARCHAR(16) NOT NULL,
			`banned` BOOL DEFAULT 0
		)
	]])

	DB.SimpleExecute([[
		CREATE TABLE IF NOT EXISTS `u_inv` (
			`user_id` INTEGER NOT NULL,
			`item_id` INTEGER NOT NULL,
			`count` INTEGER NOT NULL,
			`data` TEXT
		)
	]])

	DB.SimpleExecute([[
		CREATE TABLE IF NOT EXISTS `u_equip` (
			`user_id` INTEGER NOT NULL,
			`slot_id` INTEGER NOT NULL,
			`item_id` INTEGER NOT NULL
		)
	]])

	DB.SimpleExecute([[
		CREATE TABLE IF NOT EXISTS `u_upgrs` (
			`user_id` INTEGER NOT NULL,
			`str` INTEGER NOT NULL DEFAULT 0,
			`dex` INTEGER NOT NULL DEFAULT 0,
			`int` INTEGER NOT NULL DEFAULT 0
		)
	]])

	DB.SimpleExecute([[
		CREATE TABLE IF NOT EXISTS `u_works` (
			`user_id` INTEGER NOT NULL,
			`farmer` INTEGER NOT NULL DEFAULT 0,
			`miner` INTEGER NOT NULL DEFAULT 0,
			`forager` INTEGER NOT NULL DEFAULT 0,
			`fisher` INTEGER NOT NULL DEFAULT 0,
			`loader` INTEGER NOT NULL DEFAULT 0
		)
	]])
end


DB.SQL = {}

-- Account
DB.SQL.CheckForAccount = "SELECT COUNT(*) AS logins FROM accounts WHERE name = '%s'"
DB.SQL.NewAccount = "INSERT INTO accounts(name, password, name_registred, name_latest) VALUES ('%s', '%d', '%s', '%s')"
DB.SQL.GetAccount = "SELECT * FROM accounts WHERE name = '%s'"
DB.SQL.CheckForAccountWithPassword = "SELECT COUNT(*) AS logins FROM accounts WHERE name = '%s' AND password = '%d'"
DB.SQL.UpdateAccount = "UPDATE accounts SET level = '%d', exp = '%d', money = '%.1f', ruby = '%d', clan_id = '%d', upgrade_points = '%d' WHERE id = '%d'"

-- Inventory
DB.SQL.GetInventory = "SELECT * FROM u_inv WHERE user_id = '%d'"
DB.SQL.AddItem = "INSERT INTO u_inv(user_id, item_id, count, data) VALUES ('%d', '%d', '%d', '%s')"
DB.SQL.ClearInventory = "DELETE FROM u_inv WHERE user_id = '%d'"

-- Equipment
DB.SQL.GetEquipment = "SELECT * FROM u_equip WHERE user_id = '%d'"
DB.SQL.AddEquip = "INSERT INTO u_equip(user_id, slot_id, item_id) VALUES ('%d', '%d', '%d')"
DB.SQL.ClearEquipment = "DELETE FROM u_equip WHERE user_id = '%d'"

-- Stats
DB.SQL.NewStats = "INSERT INTO u_upgrs(user_id) VALUES ('%d')"
DB.SQL.GetStats = "SELECT * FROM u_upgrs WHERE user_id = '%d'"
DB.SQL.UpdateStats = "UPDATE u_upgrs SET str = '%d', dex = '%d', int = '%d' WHERE user_id = '%d'"

-- Stats
DB.SQL.NewWorks = "INSERT INTO u_works(user_id) VALUES ('%d')"
DB.SQL.GetWorks = "SELECT * FROM u_works WHERE user_id = '%d'"
DB.SQL.UpdateWorks = "UPDATE u_works SET farmer = '%d', miner = '%d', forager = '%d', fisher = '%d', loader = '%d' WHERE user_id = '%d'"
