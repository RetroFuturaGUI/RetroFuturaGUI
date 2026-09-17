#include "IHierarchyNode.hpp"
#include "config.hpp"
#include <vector>

std::string RetroFuturaGUI::IHierarchyNode::GetPath() const
{
    std::vector<const IHierarchyNode*> chain;

    for(const IHierarchyNode* node = this; node != nullptr; node = node->GetParent())
        chain.push_back(node);

    std::string path;

    for(uSize i = chain.size(); i > 0; --i)
    {
        path += chain[i - 1]->GetName();

        if(i > 1)
            path += '/';
    }

    return path;
}
