#Personal 2D Project derived from my 2D project made in School during Spring 2026. 

The process of adding a new mob is annoying, Something to rework later.

1. Add the monster to the monster.json creating a new role for it EX "ROLE_DOWN"
2. Add the role to the main role list in entities.h
3. Add to entitie's getRole() function to parse the new role   //Parse role on load
4. Add to level's getRoleFromInt() function to parse the new role  //Parse role on save
5. Add to monster.c's jsonArrayNum enum the enum number for the new entry in monster.json
6. Add to monster.c's getMonsterState function to parse the new state
7. Add to the monster.c's monsterThink and monsterUpdate Logic for how the monster should work
8. Add to monster.c's monsterState enum new monster state
9. Add to monster.c's monsterThink logic to dictate how it moves, otherwise it moves like trashMob
