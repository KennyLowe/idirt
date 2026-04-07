/* setlevel.c - Set a player's level in the UAF file
 * Usage: setlevel <uaf_file> <player_name> <level>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../kernel.h"

int main(int argc, char *argv[]) {
    FILE *f;
    PERSONA p;
    int found = 0;
    long pos;
    int new_level;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <uaf_file> <player_name> <level>\n", argv[0]);
        return 1;
    }

    new_level = atoi(argv[3]);

    if ((f = fopen(argv[1], "r+")) == NULL) {
        perror("fopen");
        return 1;
    }

    while (fread(&p, sizeof(PERSONA), 1, f) == 1) {
        if (strcasecmp(p.p_name, argv[2]) == 0) {
            found = 1;
            printf("Found player: %s (current level: %d)\n", p.p_name, p.p_level);
            p.p_level = new_level;
            pos = ftell(f) - sizeof(PERSONA);
            fseek(f, pos, SEEK_SET);
            fwrite(&p, sizeof(PERSONA), 1, f);
            printf("Set level to: %d\n", new_level);
            break;
        }
    }

    fclose(f);

    if (!found) {
        fprintf(stderr, "Player '%s' not found in UAF file.\n", argv[2]);
        fprintf(stderr, "The player must log in at least once first.\n");
        return 1;
    }

    return 0;
}
