#pragma once

#include "util/util.h"
#include "utils.h"


namespace AT
{

    // Definition structure for a minimal node
    // minimalNodeRegistry_group
    struct minimal_node_definition
    {
        std::string                                                                     name;           // Name of the node
        std::string                                                                     display_name;   // Display name of the node
        std::string                                                                     description;    // Description of the node's function
        std::string                                                                     category;       // Category for organization
        std::vector<pin_data_type>                                                      inputs;         // Input pin data types
        std::vector<pin_data_type>                                                      outputs;        // Output pin data types
        std::function<std::vector<function_value>(const std::vector<function_value>&)>  operation;      // Processing function
    };


    // Singleton registry for minimal node definitions
    // minimalNodeRegistry_group
    // This class maintains a registry of all available minimal nodes that can be used
    // in the Visual Programming editor. It provides methods to register, retrieve,
    // and categorize node definitions.
    class minimal_node_registry
    {
    public:
            // Gets the singleton instance of the registry
            // Reference to the singleton instance
        static minimal_node_registry& get()
        {
            static minimal_node_registry instance;
            return instance;
        }

        // Registers a new node definition
        // def The node definition to register
        void register_node(const minimal_node_definition &def);

        // Retrieves a node definition by name
        // name The name of the node to retrieve
        // Pointer to the node definition, or nullptr if not found
        const minimal_node_definition *get_node(const std::string &name) const;

        // Gets all registered node definitions
        // Const reference to the map of all node definitions
        const std::unordered_map<std::string, minimal_node_definition> &get_all_nodes() const;

        // Gets all node names in a specific category
        // category The category to filter by
        // Vector of node names in the specified category
        std::vector<std::string> get_nodes_by_category(const std::string &category) const;

    private:

        // Private constructor for singleton pattern
        minimal_node_registry();

        // Initializes standard nodes (math, logic, string operations)
        void initialize_standard_nodes();

        std::unordered_map<std::string, minimal_node_definition>  m_nodes; // Storage for node definitions
    };

}
