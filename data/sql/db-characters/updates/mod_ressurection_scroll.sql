--
DROP TABLE IF EXISTS mod_ress_scroll_accounts;
CREATE TABLE mod_ress_scroll_accounts (
    AccountId INT UNSIGNED NOT NULL,
    EndDate INT UNSIGNED NOT NULL,
	Expired TINYINT NOT NULL DEFAULT 0,
    PRIMARY KEY (AccountId)
);
