#ifndef		BINARYTREE_HPP
# define	BINARYTREE_HPP

#include	<cstdint>

namespace	Yo
{
  template <typename T>
  class		BinaryTree
  {
  public:
    BinaryTree(uint32_t p_key, T p_value)
    {
      m_left = nullptr;
      m_right = nullptr;
      m_key = p_key;
      m_value = p_value;
    }

    ~BinaryTree()
    {
      if (m_left)
	delete (m_left);
      if (m_right)
	delete (m_right);
    }
    
    BinaryTree	*Find(uint32_t p_key) const
    {
      BinaryTree *l_parent = nullptr;
      return (Find(p_key, this, &l_parent));
    }
    
    bool	Remove(uint32_t p_key)
    {
      return (Remove(p_key, this));
    }
    
    BinaryTree	*Insert(uint32_t p_key, T p_value)
    {
      return (Insert(new BinaryTree(p_key, p_value), this));
    }
  private:
    BinaryTree	*Find(uint32_t p_key, const BinaryTree *p_node, BinaryTree **p_parent) const
    {
      if (p_node == nullptr)
	return (nullptr);
      if (p_node->m_key == p_key)
	return (p_node);
      if (p_node->m_left != nullptr && p_node->m_left->m_value < p_key)
	return (Find(p_key, p_node->m_left, p_node));
      return (Find(p_key, p_node->m_right, p_node));
    }

    bool	Remove(uint32_t p_key, BinaryTree *p_node)
    {
      BinaryTree	*l_parent = nullptr;
      BinaryTree	*l_toRemove = Find(p_key, p_node, l_parent);

      if (l_toRemove == nullptr)
	return (false);
      if (l_parent != nullptr)
	{
	  if (l_parent->m_left == l_toRemove)
	    l_parent->m_left = nullptr;
	  else if (l_parent->m_right == l_toRemove)
	    l_parent->m_right= nullptr;
	}
      Insert(p_node, l_toRemove->m_left);
      Insert(p_node, l_toRemove->m_right);
      return (true);
    }
    
    BinaryTree	*Insert(BinaryTree *p_node, BinaryTree *p_toInsert)
    {
      if (p_node == nullptr)
	return (p_toInsert);
      if (p_toInsert->m_key < p_node->m_key)
	p_node->left = Insert(p_node->left, p_toInsert);
      else
	p_node->right = Insert(p_node->right, p_toInsert);
      return (p_node);
    }
    BinaryTree	*m_left;
    BinaryTree	*m_right;
    uint32_t	m_key;
    T		m_value;
  };
}
#endif
