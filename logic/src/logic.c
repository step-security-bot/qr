#include <logic.h>
#include <stdfunc.h>

enum item_t g_playerInventory[ PLAYER_MAX_ITEM_COUNT ];
uint32_t    g_playerInventoryItemCount = 0;
int32_t     g_playerHealth             = 2;
uint32_t    g_playerExperience         = 0;
uint32_t    g_playerPosition           = 0;

#if defined( CLIENT ) || defined( SERVER )

uint32_t g_secondPlayerPosition        = 0;

#endif // CLIENT || SERVER

uint32_t      g_guardiansPositions[ MAX_GUARDIANS_ON_MAP ];
uint32_t      g_followMonstersPositions[ MAX_MONSTERS_ON_MAP ];
uint32_t      g_randomMonstersPositions[ MAX_MONSTERS_ON_MAP ];
uint32_t      g_guardianHealth     = GUARDIAN_MAX_HEALTH;
uint32_t      g_monsterHealth      = MONSTER_MAX_HEALTH;
uint32_t      g_keyMonsterHealth   = KEY_MONSTER_MAX_HEALTH;
const char    g_monsters[]         = { FOLLOW_MONSTER, RANDOM_MONSTER };
const char    g_keyMonsters[]      = { KEY_MONSTER };
uint32_t      g_guardiansLeft      = 0;
uint32_t      g_followMonstersLeft = 0;
uint32_t      g_randomMonstersLeft = 0;

const char* g_emptyMap =
"\
+------+     +-------+   ##                  ######                             \
|......|  ###........|    #             +----.----+        #                    \
|.......###  |.......|    #             |.........|        #                    \
|......|  #  |........###### ###########..........|     ####                    \
|......|     |.......|      #           |.........|     #                       \
+------+     |.......|            +-----+.........|     #                       \
|......|     +-------+            |................#####                        \
|......+-----+                    |...............|                             \
>.............####################................|                             \
+------------+                    +---------------+                           \n\
";

char g_map[] =
"\
+------+     +-------+   M#                  K####T                             \
|....K.|  ###}.......|    #             +----/----+        C                    \
|.@.....###  |.......|    #             |.........|        #                    \
|......|  #  |........###### ###########}.........|     W##M                    \
|......|     |.M.....|      K           |.........|     #                       \
+------+     |...C...|            +-----+C........|     #                       \
|.G....|     +-------+            |.....C.M.......{#####                        \
|......+-----+                    |..........M....|                             \
>............}K###################}....W..........|                             \
+------------+                    +---------------+                           \n\
";

char g_vision[] = "00000 00000 00000 00000 00000 0";

void initMap( void ) {
    for ( uint32_t _mapCellIndex = 0; _mapCellIndex < sizeof( g_map ); _mapCellIndex++ ) {
        switch ( g_map[ _mapCellIndex ] ) {
            case PLAYER:
            {
                g_playerPosition = _mapCellIndex;

            #if defined( CLIENT ) || defined( SERVER )

                if ( g_playerPosition ) {
                    g_secondPlayerPosition = _mapCellIndex;
                }

            #endif // CLIENT || SERVER

                break;
            }

            // Spawn opponents
            case GUARDIAN:
            {
                g_guardiansPositions[ g_guardiansLeft ] = _mapCellIndex;
                g_guardiansLeft++;

                break;
            }

            case MONSTER:
            {
                g_map[ _mapCellIndex ] = g_monsters[ Rand() % 2 ];

                if ( g_map[ _mapCellIndex ] == RANDOM_MONSTER ) {
                    g_randomMonstersPositions[ g_randomMonstersLeft ] = _mapCellIndex;
                    g_randomMonstersLeft++;

                } else {
                    g_followMonstersPositions[ g_followMonstersLeft ] = _mapCellIndex;
                    g_followMonstersLeft++;
                }

                break;
            }

            case MONSTER_WITH_A_KEY:
            {
                g_map[ _mapCellIndex ] = g_keyMonsters[ Rand() % 1 ];

                break;
            }
        };
    }

#if defined( CLIENT ) || defined( SERVER )

    if ( !g_secondPlayerPosition ) {
        g_secondPlayerPosition = g_playerPosition;
    }

#endif // CLIENT || SERVER
}

void initInventory( enum item_t _item ) {
    g_playerInventoryItemCount = ( _item == EMPTY )
        ? 0
        : PLAYER_MAX_ITEM_COUNT;
    uint32_t l_inventoryCellIndex = PLAYER_MAX_ITEM_COUNT;

    while ( l_inventoryCellIndex-- ) {
        g_playerInventory[ l_inventoryCellIndex ] = _item;
    }
}

void getOverview( const uint32_t _playerPosition ) {
    uint32_t l_overviewCell = ( _playerPosition - ( 80 * 2 ) - 2 );
    uint32_t l_counter      = 0;

    for ( uint32_t _overviewAreaIndex = 1; _overviewAreaIndex < 6; _overviewAreaIndex++ ) {
        if ( ( l_overviewCell % 80 ) > ( ( _playerPosition % 80 ) + 2 ) ) {
            l_overviewCell++;

        } else if ( ( l_overviewCell % 80 ) < ( ( _playerPosition % 80 ) - 2 ) ) {
            l_overviewCell--;
        }

        for ( uint32_t _mapCellIndex = l_overviewCell; _mapCellIndex < ( l_overviewCell + 4 + 1 ); _mapCellIndex++ ) {
            g_vision[ l_counter ] = g_map[ _mapCellIndex ];
            l_counter++;
        }

        g_vision[ l_counter ] = '\n';
        l_counter++;

        l_overviewCell = ( _playerPosition - ( 80 * ( 2 - _overviewAreaIndex ) ) - 2 );
    }

    g_vision[ l_counter ] = '\0';
}

int32_t getPlayerInventoryPlaceOf( const enum item_t _item ) {
    for ( uint32_t _inventoryCellIndex = 0; _inventoryCellIndex < PLAYER_MAX_ITEM_COUNT; _inventoryCellIndex++ ) {
        if ( g_playerInventory[ _inventoryCellIndex ] == _item ) {
            return ( (int32_t)_inventoryCellIndex );
        }
    }

    return ( INT8_MIN );
}

bool inventoryAdd( const enum item_t _item, int32_t _itemIndex ) {
    // if ( g_playerInventoryItemCount >= PLAYER_MAX_ITEM_COUNT ) {
    //     return (false);

    // } else if (
    //     ( _itemIndex != INT8_MIN ) &&
    //     ( g_playerInventory[ _itemIndex ] != EMPTY )
    // ) {
    //     return (false);

    // } else if ( _itemIndex == INT8_MIN ) {
    //     _itemIndex = getPlayerInventoryPlaceOf( EMPTY );
    // }

    if (
        ( g_playerInventoryItemCount >= PLAYER_MAX_ITEM_COUNT ) ||
        (
            ( _itemIndex != INT8_MIN ) &&
            ( g_playerInventory[ _itemIndex ] != EMPTY )
        )
    ) {
        return (false);
    }

    const int32_t l_emptyIndex = ( _itemIndex != INT8_MIN )
        ? _itemIndex
        : getPlayerInventoryPlaceOf( EMPTY );

    g_playerInventory[ l_emptyIndex ] = _item;
    g_playerInventoryItemCount++;

    return (true);
}

bool usePlayerItem( const enum item_t _item ) {
    int l_itemPlace = getPlayerInventoryPlaceOf( _item );

    if ( ( g_playerInventoryItemCount < 1 ) || ( l_itemPlace < 0 ) ) {
        return (false);
    }

    g_playerInventory[ l_itemPlace ] = EMPTY;
    g_playerInventoryItemCount--;

    return (true);
}

uint32_t move( const char _who, uint32_t _currentPosition, const int32_t _offset ) {
    g_map[ _currentPosition ] = g_emptyMap[ _currentPosition ];
    _currentPosition += _offset;
    g_map[ _currentPosition ] = _who;

    return ( _currentPosition );
}

bool doPlayerMove( const uint32_t _offset, const bool isSecondPlayer ) {
    uint32_t* l_playerPosition;

#if defined( CLIENT ) || defined( SERVER )

    if ( isSecondPlayer ) {
        l_playerPosition = &g_secondPlayerPosition;

    } else {
        l_playerPosition = &g_playerPosition;
    }

#else // CLIENT || SERVER

    l_playerPosition = &g_playerPosition;

#endif // CLIENT || SERVER

    const char l_cellToMove = g_map[ *l_playerPosition + _offset ];

    for ( uint32_t _monsterIndex = 0; _monsterIndex < sizeof( g_monsters ); _monsterIndex++ ) {
        if ( l_cellToMove == g_monsters[ _monsterIndex ] ) {
            *l_playerPosition = fight( PLAYER, *l_playerPosition, _offset );

            return ( *l_playerPosition );
        }
    }

    for ( uint32_t _keyMonsterIndex = 0; _keyMonsterIndex < sizeof( g_keyMonsters ); _keyMonsterIndex++ ) {
        if ( l_cellToMove == g_keyMonsters[ _keyMonsterIndex ] ) {
            *l_playerPosition = fight( PLAYER, *l_playerPosition, _offset );

            return ( *l_playerPosition );
        }
    }

    switch ( g_map[ *l_playerPosition + _offset ] ) {
        case FLOOR:
        case BRIDGE:
        {
            *l_playerPosition = move( PLAYER, *l_playerPosition, _offset );

            break;
        }

        case DOOR_LEFT:
        case DOOR_RIGHT:
        case DOOR_MIDDLE:
        {
            if ( usePlayerItem( KEY ) ){
                *l_playerPosition = move( PLAYER, *l_playerPosition, _offset );
            }

            break;
        }

        case LADDER_LEFT:
        case LADDER_RIGHT:
        {
            if ( !g_guardiansLeft ){
                return ( false );
            }

            break;
        }

        case GUARDIAN:
        {
            *l_playerPosition = fight( PLAYER, *l_playerPosition, _offset );

            return ( *l_playerPosition );
        }

        case KEY:
        {
            if ( inventoryAdd( KEY, INT8_MIN ) ) {
                *l_playerPosition = move( PLAYER, *l_playerPosition, _offset );
                // PlaySoundA( "SystemExit", NULL, SND_SYNC );
            }

            break;
        }

        case HEALTH:
        {
            if ( inventoryAdd( HEALTH, INT8_MIN ) ) {
                *l_playerPosition = move( PLAYER, *l_playerPosition, _offset );
                // PlaySoundA( "SystemExit", NULL, SND_SYNC );
            }

            break;
        }

        case ATTACK:
        {
            if ( inventoryAdd( ATTACK, INT8_MIN ) ) {
                *l_playerPosition = move( PLAYER, *l_playerPosition, _offset );
                // PlaySoundA( "SystemExit", NULL, SND_SYNC );
            }

            break;
        }

        case DEFENSE:
        {
            if ( inventoryAdd( DEFENSE, INT8_MIN ) ) {
                *l_playerPosition = move( PLAYER, *l_playerPosition, _offset );
                // PlaySoundA( "SystemExit", NULL, SND_SYNC );
            }

            break;
        }

        case CHEST:
        {
            const enum item_t l_chestItems[] = { HEALTH, ATTACK, DEFENSE };

            if ( inventoryAdd( l_chestItems[ ( Rand() % sizeof( l_chestItems ) ) ], INT8_MIN ) ) {
                *l_playerPosition = move( PLAYER, *l_playerPosition, _offset );
                // PlaySoundA( "SystemExit", NULL, SND_SYNC );
            }

            break;
        }

        case TREASURE:
        {
            g_playerExperience += ( ( Rand() % MAX_EXPERIENCE_FROM_TREASURE ) + 1 );

            *l_playerPosition = move( PLAYER, *l_playerPosition, _offset );
            // PlaySoundA( "SystemExit", NULL, SND_SYNC );

            break;
        }
    };

    return (true);
}

static void _randomMove( const char _who, uint32_t* _currentPosition ) {
    enum direction_t l_offset;

    switch ( Rand() % 4 ) {
        case 1:
        {
            l_offset = DOWN;

            break;
        }

        case 2:
        {
            l_offset = UP;

            break;
        }

        case 3:
        {
            l_offset = LEFT;

            break;
        }

        case 0:
        {
            l_offset = RIGHT;

            break;
        }

        default:
        {
            l_offset = STAY;
        }
    };

    if (
        ( g_map[ *_currentPosition + l_offset ] == FLOOR ) ||
        ( g_map[ *_currentPosition + l_offset ] == BRIDGE )
    ) {
        *_currentPosition = move( _who, *_currentPosition, l_offset );

    } else if ( g_map[ *_currentPosition + l_offset ] == PLAYER ) {
        *_currentPosition = fight( _who, *_currentPosition, l_offset );
    }
}

static void _followMove( const char _who, uint32_t* _currentPosition ) {
    enum direction_t l_offset;
    uint32_t* l_playerPosition;

#if defined( CLIENT ) || defined( SERVER )

    if ( Rand() % 2 ) {
        l_playerPosition = &g_secondPlayerPosition;

    } else {
        l_playerPosition = &g_playerPosition;
    }

#else // CLIENT || SERVER

    l_playerPosition = &g_playerPosition;

#endif // CLIENT || SERVER

    if ( ( *_currentPosition % 80 ) < ( *l_playerPosition % 80 ) ) {
        l_offset = RIGHT;

    } else if ( ( *_currentPosition % 80 ) > ( *l_playerPosition % 80 ) ) {
        l_offset = LEFT;

    } else if ( *_currentPosition < *l_playerPosition ) {
        l_offset = DOWN;

    } else {
        l_offset = UP;
    }

    if (
        ( g_map[ *_currentPosition + l_offset ] == FLOOR ) ||
        ( g_map[ *_currentPosition + l_offset ] == BRIDGE )
    ) {
        *_currentPosition = move( _who, *_currentPosition, l_offset );

    } else if ( g_map[ *_currentPosition + l_offset ] == PLAYER ) {
        *_currentPosition = fight( _who, *_currentPosition, l_offset );
    }
}

bool doOpponentMove( void ) {
    bool l_isPlayerInVision = false;

    for ( uint_fast64_t _visionCellIndex = 0; _visionCellIndex < lengthOfCString( g_vision ); _visionCellIndex++ ) {
        if ( g_vision[ _visionCellIndex ] == PLAYER ) {
            l_isPlayerInVision = true;
        }
    }

    for ( uint32_t _randomMonsterIndex = 0; _randomMonsterIndex < g_randomMonstersLeft; _randomMonsterIndex++ ) {
        _randomMove( RANDOM_MONSTER, &g_randomMonstersPositions[ _randomMonsterIndex ] );
    }

    for ( uint32_t _followMonsterIndex = 0; _followMonsterIndex < g_followMonstersLeft; _followMonsterIndex++ ) {
        if ( l_isPlayerInVision ) {
            _followMove( FOLLOW_MONSTER, &g_followMonstersPositions[ _followMonsterIndex ] );

        } else {
            _randomMove( FOLLOW_MONSTER, &g_followMonstersPositions[ _followMonsterIndex ] );
        }
    }

    for ( uint32_t _guardianIndex = 0; _guardianIndex < g_guardiansLeft; _guardianIndex++ )
        if ( l_isPlayerInVision ) {
            _followMove( GUARDIAN, &g_guardiansPositions[ _guardianIndex ] );

        } else {
            _randomMove( GUARDIAN, &g_guardiansPositions[ _guardianIndex ] );
        }

    return ( true );
}

uint32_t fight( const char _who, uint32_t _currentPosition, const int32_t _offset ) {
    const bool l_isPoweredAttack = usePlayerItem( ATTACK );

    if ( ( _who == GUARDIAN ) || ( g_map[ _currentPosition + _offset ] == GUARDIAN ) ) {
        g_playerHealth -= ( usePlayerItem( DEFENSE ) ) ? 1 : 2;

        g_guardianHealth -= (
            1 + (uint32_t)( g_playerExperience / EXPERIENCE_FOR_SUPER_DMG ) +
            (uint8_t)l_isPoweredAttack
        );

        if ( !g_guardianHealth ) {
            g_guardianHealth = GUARDIAN_MAX_HEALTH;
            g_guardiansLeft--;

            if ( _who == PLAYER ) {
                Memcpy(
                    &g_guardiansPositions,
                    pop(
                        g_guardiansPositions,
                        sizeof( g_guardiansPositions ),
                        _currentPosition + _offset
                    ),
                    sizeof( g_guardiansPositions )
                );

                return ( move( _who, _currentPosition, _offset ) );

            } else {
                Memcpy(
                    &g_guardiansPositions,
                    pop(
                        g_guardiansPositions,
                        sizeof( g_guardiansPositions ),
                        _currentPosition
                    ),
                    sizeof( g_guardiansPositions )
                );

                g_map[ _currentPosition ] = g_emptyMap[ _currentPosition ];
            }
        }

    } else {
        g_playerHealth -= 1;

        const uint16_t l_attackDamage = (
            5 + (uint32_t)( g_playerExperience / EXPERIENCE_FOR_DMG ) *
            (
                (uint8_t)l_isPoweredAttack +
                (uint8_t)l_isPoweredAttack
            )
        );

        if ( ( _who == KEY_MONSTER ) || ( g_map[ _currentPosition + _offset ] == KEY_MONSTER ) ) {
            g_keyMonsterHealth -= l_attackDamage;

            if ( !g_keyMonsterHealth ) {
                // Monster are invulnerable, while player has no empty item slot
                if ( inventoryAdd( KEY, INT8_MIN ) ) {
                    g_keyMonsterHealth = KEY_MONSTER_MAX_HEALTH;

                    return ( move( _who, _currentPosition, _offset ) );
                }
            }

        } else {
            g_monsterHealth -= l_attackDamage;

            if ( !g_monsterHealth ) {
                g_monsterHealth = MONSTER_MAX_HEALTH;

                if ( _who == PLAYER ) {
                    if ( g_map[ _currentPosition + _offset ] == RANDOM_MONSTER  ) {
                        Memcpy(
                            &g_randomMonstersPositions,
                            pop(
                                g_randomMonstersPositions,
                                sizeof( g_guardiansPositions ),
                                _currentPosition + _offset
                            ),
                            sizeof( g_randomMonstersPositions )
                        );

                        g_randomMonstersLeft--;

                    } else {
                        Memcpy(
                            &g_randomMonstersPositions,
                            pop(
                                g_randomMonstersPositions,
                                sizeof( g_guardiansPositions ),
                                _currentPosition + _offset
                            ),
                            sizeof( g_randomMonstersPositions )
                        );

                        g_followMonstersLeft--;
                    }

                    return ( move( _who, _currentPosition, _offset ) );

                } else {
                    if ( _who == RANDOM_MONSTER ) {
                        Memcpy(
                            &g_randomMonstersPositions,
                            pop(
                                g_randomMonstersPositions,
                                sizeof( g_guardiansPositions ),
                                _currentPosition
                            ),
                            sizeof( g_randomMonstersPositions )
                        );

                        g_randomMonstersLeft--;

                    } else {
                        Memcpy(
                            &g_randomMonstersPositions,
                            pop(
                                g_randomMonstersPositions,
                                sizeof( g_guardiansPositions ),
                                _currentPosition
                            ),
                            sizeof( g_randomMonstersPositions )
                        );

                        g_followMonstersLeft--;
                    }

                    g_map[ _currentPosition ] = g_emptyMap[ _currentPosition ];

                    return ( (uint32_t)1 );
                }
            }
        }
    }

    if ( !g_playerHealth ) {
        if ( usePlayerItem( HEALTH ) ) {
            g_playerHealth += 20;

            if ( g_playerHealth > PLAYER_MAX_HEALTH ) {
                g_playerHealth = PLAYER_MAX_HEALTH;
            }

        } else {
            return ( (uint32_t)0 );
        }
    }

    return ( _currentPosition );
}

void updateScreen( void ) {
    clearConsole();
    getOverview( g_playerPosition );

    print( g_vision, lengthOfCString( g_vision ) );

    print( "\nHP:", 5 );

    char* l_buffer = (char*)Malloc( lengthOfInt( g_playerHealth ) );
    Ltoa( g_playerHealth, l_buffer );
    print( l_buffer, lengthOfInt( g_playerHealth ) );
    Free( l_buffer );

    print( "\nEXP:", 6 );

    l_buffer = (char*)Malloc( lengthOfInt( g_playerExperience ) );
    Ltoa( g_playerExperience, l_buffer );
    print( l_buffer, lengthOfInt( g_playerExperience ) );
    Free( l_buffer );

    print( "\nITEMS:", 8 );

    l_buffer = (char*)Malloc( 3 );

    for ( uint32_t l_item = 0; l_item < PLAYER_MAX_ITEM_COUNT; l_item++ ) {
        if ( g_playerInventory[ l_item ] ) {
            l_buffer[ 0 ] = g_playerInventory[ l_item ];
            l_buffer[ 1 ] = ' ';
            l_buffer[ 2 ] = '\0';

            print( l_buffer, 3 );
        }
    }

    Free( l_buffer );
}
