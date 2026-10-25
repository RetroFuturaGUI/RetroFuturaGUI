#include "IHierarchyNode.hpp"
#include "config.hpp"
#include <vector>

std::string RetroFuturaGUI::IHierarchyNode::GetPath() const
{
    std::vector<const IHierarchyNode*> chain;

    for(const IHierarchyNode* node = this; node; node = node->GetParent())
        chain.push_back(node);

    std::string path;

    for(uSize i = chain.size(); 0 < i; --i)
    {
        path += chain[i - 1]->GetName();

        if(1 < i)
            path += '/';
    }

    return path;
}
