/* Copyright (c) 2010-2017 the corto developers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <corto/argparse/argparse.h>
#include <driver/tool/add/add.h>
#include <driver/tool/remove/remove.h>

static corto_bool cortotool_removeEntry(corto_file file, corto_ll list, corto_string entry) {
    corto_bool found = FALSE;
    corto_iter iter = corto_ll_iter(list);

    while (corto_iter_hasNext(&iter)) {
        corto_string str = corto_iter_next(&iter);
        if (strcmp(str, entry)) {
            fprintf(corto_fileGet(file), "%s\n", str);
        } else {
            found = TRUE;
        }
    }

    return found;
}

int cortomain(int argc, char *argv[]) {
    corto_ll silent, mute, nobuild, project, packages;
    corto_bool build = FALSE;

    CORTO_UNUSED(argc);

    corto_argdata *data = corto_argparse(
      argv,
      (corto_argdata[]){
        /* Ignore first argument */
        {"$0", NULL, NULL},
        {"--silent", &silent, NULL},
        {"--mute", &mute, NULL},
        {"--nobuild", &nobuild, NULL},

        /* Match at most one project directory */
        {"$?*", &project, NULL},

        /* At least one package must be specified */
        {"$+*", &packages, NULL},
        {NULL}
      }
    );

    /* Move to project directory */
    if (project) {
        if (corto_chdir(corto_ll_get(project, 0))) {
            goto error;
        }
    }

    if (packages) {
        corto_iter iter = corto_ll_iter(packages);
        while (corto_iter_hasNext(&iter)) {
            corto_string arg = corto_iter_next(&iter);
            corto_string package = corto_tool_lookupPackage(arg);
            if (!package) {
                package = arg; /* Try to remove by matching string */
            }

            corto_ll packages = corto_loadGetPackages();
            corto_file file = corto_fileOpen(".corto/packages.txt");
            corto_bool found = cortotool_removeEntry(file, packages, package);
            corto_fileClose(file);
            corto_loadFreePackages(packages);

            if (!found) {
                corto_error("'%s' ('%s') not found in package file", arg, package);
                goto error;
            } else {
                build = TRUE;
                printf("package '%s' removed from project\n", package);
            }
        }
    }

    if (build && !nobuild) {
        corto_load("driver/tool/build", 3, (char*[]){
          "build",
          silent ? "--silent" : "",
          mute ? "--mute" : "",
          NULL
        });
    }

    corto_argclean(data);

    return 0;
error:
    return -1;

    return 0;
}

