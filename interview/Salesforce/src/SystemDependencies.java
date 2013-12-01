import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/**
 * Salesforce - System Dependencies
 * SystemDependencies.java
 * Purpose: Automate the process of adding and removing components
 * 
 * @author Yang Sun
 * @version 1.0 09/23/2013
 */
public class SystemDependencies {

    private enum SystemCommands {
        DEPEND("DEPEND"), 
        INSTALL("INSTALL"), 
        REMOVE("REMOVE"), 
        LIST("LIST"), 
        END("END");

        private String commandName;

        private SystemCommands(String commandName) {
            this.commandName = commandName;
        }

        public boolean matchesCommand(String inputCommand) {
            return inputCommand.equals(commandName);
        }
    }

    Map<String, Set<String>> dependencies;
    Set<String> installed;

    public SystemDependencies() {
        dependencies = new HashMap<String, Set<String>>();
        installed = new HashSet<String>();
    }

    /**
     * Process the DEPEND command, add dependencies into the Map
     * @param cmdLine the user input command
     */
    public void processDEPEND(String[] cmdLine) {
        // add new entry if item didn't appear before
        if (!dependencies.containsKey(cmdLine[1])) {
            dependencies.put(cmdLine[1], new HashSet<String>());
        }
        
        for (int i = 2; i < cmdLine.length; i++) {
            // if two items depend on each other, ignore the later command
            if (dependencies.containsKey(cmdLine[i]) 
                    && dependencies.get(cmdLine[i]).contains(cmdLine[1])) {
                System.out.println("    " + cmdLine[i] + " depends on " 
                    + cmdLine[1] + ". Ignoring command.");
            } 
            
            // valid dependency command - add
            else {
                dependencies.get(cmdLine[1]).add(cmdLine[i]);
            }
        }
    }

    /**
     * Process the INSTALL command, add installed item into the SET.
     * @param packageName the package being installed
     */
    public void processINSTALL(String packageName) {
        // the item is already installed before
        if (installed.contains(packageName)) {
            System.out.println("    " + packageName + " is already installed.");
            return;
        }

        // there is no dependencies at all for the package installation
        if (!dependencies.containsKey(packageName)) {
            dependencies.put(packageName, new HashSet<String>());
        }

        for (String target : dependencies.get(packageName)) {
            // install the dependency if not yet installed
            if (!installed.contains(target)) {
                System.out.println("    Installing " + target);
                installed.add(target);
            }
        }
        
        // finally install the package
        System.out.println("    Installing " + packageName);
        installed.add(packageName);
    }

    /**
     * Process the REMOVE command, remove the item from installed SET
     * @param packageName the package being removed
     */
    public void processREMOVE(String packageName) {
        // the package is not installed in the system
        if (!installed.contains(packageName)) {
            System.out.println("    " + packageName + " is not installed.");
            return;
        }

        // the package cannot be removed due to dependency issue
        if (hasDependencies(packageName)) {
            System.out.println("    " + packageName + " is still needed.");
            return;
        }

        // keep the reference to the package dependencies for later use
        Set<String> subPackages = dependencies.get(packageName);

        // remove the package from installed SET and dependencies MAP
        System.out.println("    Removing " + packageName);
        installed.remove(packageName);
        dependencies.remove(packageName);

        // check if each of the package dependency is still being used by other packages
        // remove as well if no longer used
        for (String subPackage : subPackages) {
            if (hasDependencies(subPackage)) {
                continue;
            }
            System.out.println("    Removing " + subPackage);
            installed.remove(subPackage);
            dependencies.remove(subPackage);
        }
    }

    /**
     * Process the LIST command, print out all the installed packages
     */
    public void processLIST() {
        for (String packageName : installed) {
            System.out.println("    " + packageName);
        }
    }

    /**
     * Check if the package is still being used by other packages 
     * @param targetPackage the dependency package name we are looking for
     * @return true if the targetPackage is still a dependency of any other packages
     */
    private boolean hasDependencies(String targetPackage) {
        for (Set<String> dependentPackages : dependencies.values()) {
            // HashSet takes O(1) to search for an item
            if (dependentPackages.contains(targetPackage)) {
                return true;
            }
        }
        return false;
    }

    public static void main(String[] args) throws IOException {
        SystemDependencies dependencyProcessor = new SystemDependencies();
        BufferedReader br = null;
        try {
            br = new BufferedReader(new InputStreamReader(System.in));
            String[] cmdLine;
            do {
                String line = br.readLine();
                System.out.println(line);
                cmdLine = line.split(" +");
                if (cmdLine.length == 0) {
                    continue;
                }
                if (SystemCommands.DEPEND.matchesCommand(cmdLine[0])) {
                    if (cmdLine.length < 3) {
                        System.out.println("    is invalid command...");
                        System.out.println("    DEPEND item1 item2 [item3 ...]");
                        continue;
                    }
                    dependencyProcessor.processDEPEND(cmdLine);
                } else if (SystemCommands.INSTALL.matchesCommand(cmdLine[0])) {
                    if (cmdLine.length < 2) {
                        System.out.println("    is invalid command...");
                        System.out.println("    INSTALL item1");
                        continue;
                    }
                    dependencyProcessor.processINSTALL(cmdLine[1]);
                } else if (SystemCommands.REMOVE.matchesCommand(cmdLine[0])) {
                    if (cmdLine.length < 2) {
                        System.out.println("    is invalid command...");
                        System.out.println("    REMOVE item1");
                        continue;
                    }
                    dependencyProcessor.processREMOVE(cmdLine[1]);
                } else if (SystemCommands.LIST.matchesCommand(cmdLine[0])) {
                    dependencyProcessor.processLIST();
                }
            } while (cmdLine.length == 0 || !SystemCommands.END.matchesCommand(cmdLine[0]));
        } finally {
            if (br != null)
                br.close();
        }
    }
}
