-- Put only sql data in this file (insert, update, replace into, delete etc...).
-- If you don't use this database, then delete this file.
-- If no data, just delete this file.


DELETE FROM `command` WHERE name IN ('npc_scale', 'npc_scale set', 'npc_scale reset', 'gob_scale', 'gob_scale set', 'gob_scale reset');

INSERT INTO `command` (`name`, `security`, `help`) VALUES 
('npc_scale', 3, 'Syntax: .npc_objscale $subcommand\nType .help creature_objscale to see a list of subcommands\nor .help xp $subcommand to see info on the subcommand.'),
('npc_scale set', 3, 'Syntax: .npc_objscale set $scale\nSet scale of selected creature.'),
('npc_scale reset', 3, 'Syntax: .npc_objscale reset \nReset scale of selected creature.'),
('gob_scale', 3, 'Syntax: .gob_objscale $subcommand\nType .help gob_objscale to see a list of subcommands\nor .help xp $subcommand to see info on the subcommand.'),
('gob_scale set', 3, 'Syntax: .gob_objscale set $identifier $scale\nSet scale of selected GameObject.'),
('gob_scale reset', 3, 'Syntax: .gob_objscale set $identifier \nReset scale of selected GameObject.');
