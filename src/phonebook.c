#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rq_term.h"
#include "rq_phonebook.h"

typedef enum {
    CONTACT_MENU_LIST,
    CONTACT_MENU_FIND,
    CONTACT_MENU_CREATE,
    CONTACT_MENU_REMOVE,
    CONTACT_MENU_LEN
} MENU;

char *menu_to_name(MENU menu) {
    switch (menu)
    {
    case CONTACT_MENU_LIST:
        return "List all contacts";

    case CONTACT_MENU_FIND:
        return "Find a contact";
    
    case CONTACT_MENU_CREATE:
        return "Create a contact";

    case CONTACT_MENU_REMOVE:
        return "Remove a contact";

    default:
        panic("This menu does not exist", RQ_ERROR_ENUM_VIOLATION); // Should never call this with anything else
        return "";
    };
}

void loop(PhoneBook *pb) {
    char *menus[] = {menu_to_name(CONTACT_MENU_LIST), menu_to_name(CONTACT_MENU_FIND), menu_to_name(CONTACT_MENU_CREATE), menu_to_name(CONTACT_MENU_REMOVE)};
    uint selected = select_from_menu("Select a menu:\r\n", menus, CONTACT_MENU_LEN);
    while (selected != CONTACT_MENU_LEN) {
        switch (selected)
        {
            case CONTACT_MENU_LIST:
                pb_print(pb);
                break;

            case CONTACT_MENU_FIND:
                pb_find(pb);
                break;
            
            case CONTACT_MENU_CREATE:
                pb_create(pb);
                break;

            case CONTACT_MENU_REMOVE:
                pb_remove(pb);
                break;
            
            default:
                break;
        }
        selected = select_from_menu("Select a menu:\r\n", menus, CONTACT_MENU_LEN);
    }
}

int main(void) {
    FILE *f = fopen("nums", "r+b");
    if (f == NULL) {
        f = fopen("nums", "w+b");
    }
    if (f == NULL) { // A fucky wucky happened
        panic("Couldn't read file", RQ_ERROR_FILE_READING);
    }

    PhoneBook *pb = malloc(sizeof(PhoneBook));
    if (pb == NULL) {
        panic("Failed to allocate enough memory for Phone Book", RQ_ERROR_NOT_ENOUGH_MEMORY);
    }
    pb_init(pb, f);

    loop(pb);

    pb_destroy(pb, f);
    fclose(f);

    return 0;
}
