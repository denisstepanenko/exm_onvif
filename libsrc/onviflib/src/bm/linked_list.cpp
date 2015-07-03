/***************************************************************************************
 *
 *  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 *
 *  By downloading, copying, installing or using the software you agree to this license.
 *  If you do not agree to this license, do not download, install, 
 *  copy or use the software.
 *
 *  Copyright (C) 2010-2014, Happytimesoft Corporation, all rights reserved.
 *
 *  Redistribution and use in binary forms, with or without modification, are permitted.
 *
 *  Unless required by applicable law or agreed to in writing, software distributed 
 *  under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 *  CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
 *  language governing permissions and limitations under the License.
 *
****************************************************************************************/

#include "sys_inc.h"
#include "linked_list.h"

/************************************************************************************\

\************************************************************************************/
LINKED_LIST* h_list_create (BOOL bNeedMutex)
{
	LINKED_LIST* p_linked_list;

	p_linked_list = (LINKED_LIST*) malloc (sizeof (LINKED_LIST));

	if (p_linked_list == NULL)
	{
		return NULL;
	}
		
	p_linked_list->p_first_node = NULL;
	p_linked_list->p_last_node = NULL;

	if(bNeedMutex)
		p_linked_list->list_semMutex = sys_os_create_mutex();
	else
		p_linked_list->list_semMutex = NULL;

	return p_linked_list;
}
/************************************************************************************/
void get_ownership(LINKED_LIST* p_linked_list)
{
	if(p_linked_list->list_semMutex)
	{
		sys_os_mutex_enter(p_linked_list->list_semMutex);
	}
}
/************************************************************************************/
void giveup_ownership(LINKED_LIST* p_linked_list)
{
	if(p_linked_list->list_semMutex)
	{
		sys_os_mutex_leave(p_linked_list->list_semMutex);
	}
}
/************************************************************************************/
void h_list_free_container (LINKED_LIST* p_linked_list)
{
	LINKED_NODE* p_node;
	LINKED_NODE* p_next_node;

	if (p_linked_list == NULL)	return;

	get_ownership(p_linked_list);

	p_node = p_linked_list->p_first_node;

	while (p_node != NULL) 
	{
		void * p_free;

		p_next_node = p_node->p_next;
		
		p_free = p_node->p_data;

		if(p_free != NULL)
			free(p_free);
			
		free (p_node);

		p_node = p_next_node;		
	}

	giveup_ownership(p_linked_list);

	if(p_linked_list->list_semMutex)
	{
		sys_os_destroy_sig_mutx(p_linked_list->list_semMutex);
	}

	free (p_linked_list);
}

/************************************************************************************/
void h_list_free_all_node (LINKED_LIST* p_linked_list)
{
	LINKED_NODE* p_node;
	LINKED_NODE* p_next_node;

	if (p_linked_list == NULL)	return;

	get_ownership(p_linked_list);

	p_node = p_linked_list->p_first_node;

	while (p_node != NULL) 
	{
		p_next_node = p_node->p_next;
		
		if(p_node->p_data != NULL)
			free(p_node->p_data);
			
		free (p_node);

		p_node = p_next_node;		
	}
	
	p_linked_list->p_first_node = NULL;
	p_linked_list->p_last_node = NULL;

	giveup_ownership(p_linked_list);	
}

/************************************************************************************/
BOOL h_list_add_at_front (LINKED_LIST* p_linked_list, void* p_item)
{
	LINKED_NODE* p_node;
	LINKED_NODE* p_next_node;

	if (p_linked_list == NULL)	return (FALSE);

	if (p_item == NULL)	return (FALSE);
		
	p_node = (LINKED_NODE*) malloc (sizeof (LINKED_NODE));

	if (p_node == NULL)	return (FALSE);
		
	p_node->p_next = NULL;
	p_node->p_previous = NULL;
	p_node->p_data = p_item;

	get_ownership(p_linked_list);

	if (p_linked_list->p_first_node == NULL)
	{
		p_linked_list->p_first_node = p_node;
		p_linked_list->p_last_node = p_node;

		p_node->p_previous = NULL;
		p_node->p_next = NULL;
	}
	else
	{
		p_next_node = p_linked_list->p_first_node;

		p_node->p_next = p_next_node;
		p_node->p_previous = NULL;

		p_next_node->p_previous = p_node;
		p_linked_list->p_first_node = p_node;
	}

	giveup_ownership(p_linked_list);
	return (TRUE);
}
/************************************************************************************/
void h_list_remove_from_front (LINKED_LIST* p_linked_list)
{
	LINKED_NODE* p_node_to_remove;

	if (p_linked_list == NULL)	return;
		
	get_ownership(p_linked_list);

	p_node_to_remove = p_linked_list->p_first_node;

	if (p_node_to_remove == NULL) 
	{
		giveup_ownership(p_linked_list);
		return;
	}

	if (p_linked_list->p_first_node == p_linked_list->p_last_node)
	{
		p_linked_list->p_first_node = NULL;
		p_linked_list->p_last_node = NULL;
	}
	else
	{
		p_linked_list->p_first_node = p_node_to_remove->p_next;
		p_linked_list->p_first_node->p_previous = NULL;
	}

	free (p_node_to_remove);
	giveup_ownership(p_linked_list);
}
/************************************************************************************/
void h_list_remove_from_front_no_lock (LINKED_LIST* p_linked_list)
{
	LINKED_NODE* p_node_to_remove;

	if (p_linked_list == NULL)	return;

	p_node_to_remove = p_linked_list->p_first_node;

	if (p_node_to_remove == NULL) 
	{
		return;
	}

	if (p_linked_list->p_first_node == p_linked_list->p_last_node)
	{
		p_linked_list->p_first_node = NULL;
		p_linked_list->p_last_node = NULL;
	}
	else
	{
		p_linked_list->p_first_node = p_node_to_remove->p_next;
		p_linked_list->p_first_node->p_previous = NULL;
	}

	free (p_node_to_remove);
}
/************************************************************************************/
BOOL h_list_add_at_back (LINKED_LIST* p_linked_list, void* p_item)
{
	LINKED_NODE* p_node;
	LINKED_NODE* p_previous_node;

	if (p_linked_list == NULL)	return (FALSE);
		
	if (p_item == NULL)	return (FALSE);
		
	p_node = (LINKED_NODE*) malloc (sizeof (LINKED_NODE));

	if (p_node == NULL)	return (FALSE);

	p_node->p_next = NULL;
	p_node->p_previous = NULL;
	p_node->p_data = (void*) p_item;

	get_ownership(p_linked_list);

	if (p_linked_list->p_last_node == NULL)
	{
		p_linked_list->p_last_node = p_node;
		p_linked_list->p_first_node = p_node;
	
		p_node->p_next = NULL;
		p_node->p_previous = NULL;
	}
	else
	{
		p_previous_node = p_linked_list->p_last_node;

		p_node->p_next = NULL;
		p_node->p_previous = p_previous_node;

		p_previous_node->p_next = p_node;

		p_linked_list->p_last_node = p_node;
	}

	giveup_ownership(p_linked_list);

	return (TRUE);
}
/************************************************************************************/
void h_list_remove_from_back (LINKED_LIST* p_linked_list)
{
	LINKED_NODE* p_node_to_remove;

	p_linked_list = p_linked_list;

	if (p_linked_list == NULL) return;

	get_ownership(p_linked_list);

	if (p_linked_list->p_last_node == NULL)	
	{
		giveup_ownership(p_linked_list);
		return;
	}
		
	if (p_linked_list->p_first_node == p_linked_list->p_last_node)
	{
		p_node_to_remove = p_linked_list->p_first_node;

		p_linked_list->p_first_node = NULL;
		p_linked_list->p_last_node = NULL;

		free (p_node_to_remove);

		giveup_ownership(p_linked_list);
		return;
	}
		
	p_node_to_remove = p_linked_list->p_last_node;
	
	p_linked_list->p_last_node = p_node_to_remove->p_previous;

	p_linked_list->p_last_node->p_next = NULL;
	
	free (p_node_to_remove);

	p_node_to_remove = NULL;
	
	giveup_ownership(p_linked_list);
}
/************************************************************************************/
BOOL h_list_remove(LINKED_LIST* p_linked_list,LINKED_NODE * p_node)
{
	LINKED_NODE* p_previous_node;
	LINKED_NODE* p_next_node;

	if ((p_linked_list == NULL) || (p_node == NULL))
	{
		return (FALSE);
	}

	p_previous_node = p_node->p_previous;
	
	p_next_node = p_node->p_next;
	
	if (p_previous_node != NULL)
		p_previous_node->p_next = p_next_node;
	else
		p_linked_list->p_first_node = p_next_node;


	if (p_next_node != NULL)
		p_next_node->p_previous = p_previous_node;
	else
		p_linked_list->p_last_node = p_previous_node;
	
	free (p_node);

	return (TRUE);				
}
/************************************************************************************/
BOOL h_list_remove_data(LINKED_LIST* p_linked_list,void * p_data)
{
	LINKED_NODE* p_previous_node;
	LINKED_NODE* p_next_node;
	LINKED_NODE* p_node;

	if ((p_linked_list == NULL) || (p_data == NULL))
		return (FALSE);

	get_ownership(p_linked_list);

	p_node = p_linked_list->p_first_node;

	while(p_node != NULL)
	{
		if(p_data == p_node->p_data)break;
		p_node = p_node->p_next;
	}

	if(p_node == NULL)
	{
		giveup_ownership(p_linked_list);
		return FALSE;
	}

	p_previous_node = p_node->p_previous;
	
	p_next_node = p_node->p_next;
	
	if (p_previous_node != NULL)
		p_previous_node->p_next = p_next_node;
	else
		p_linked_list->p_first_node = p_next_node;


	if (p_next_node != NULL)
		p_next_node->p_previous = p_previous_node;
	else
		p_linked_list->p_last_node = p_previous_node;
	
	free (p_node);

	giveup_ownership(p_linked_list);

	return (TRUE);				
}
/************************************************************************************/
unsigned int h_list_get_number_of_nodes (LINKED_LIST* p_linked_list)
{
	unsigned int number_of_nodes;
	LINKED_NODE* p_node;

	get_ownership(p_linked_list);

	p_node = p_linked_list->p_first_node;

	number_of_nodes = 0;
	while (p_node !=  NULL)
	{
		++ number_of_nodes;
		p_node = p_node->p_next;
	}

	giveup_ownership(p_linked_list);

	return (number_of_nodes);
}

/************************************************************************************/
BOOL h_list_insert(LINKED_LIST * p_linked_list,LINKED_NODE * p_pre_node,void *p_item)
{
	if((p_linked_list == NULL) || (p_item == NULL))
		return FALSE;
	
	if(p_pre_node == NULL)
		h_list_add_at_front(p_linked_list, p_item);
	else
	{
		if(p_pre_node->p_next == NULL)
			h_list_add_at_back(p_linked_list, p_item);
		else
		{
			LINKED_NODE * p_node = (LINKED_NODE *)malloc(sizeof(LINKED_NODE));
			if (NULL == p_node)
			{
			    return FALSE;
			}
		
			get_ownership(p_linked_list);

			p_node->p_data = p_item;
			p_node->p_next = p_pre_node->p_next;
			p_node->p_previous = p_pre_node;
			p_pre_node->p_next->p_previous = p_node;
			p_pre_node->p_next = p_node;
		
			giveup_ownership(p_linked_list);
		}
	}

	return TRUE;
}
/***********************************************************************\
\***********************************************************************/
LINKED_NODE * h_list_lookup_start(LINKED_LIST * p_linked_list)
{
	if(p_linked_list == NULL)
		return NULL;

	get_ownership(p_linked_list);

	if(p_linked_list->p_first_node)
	{
		return p_linked_list->p_first_node;
	}

	return NULL;
}

LINKED_NODE * h_list_lookup_next(LINKED_LIST * p_linked_list, LINKED_NODE * p_node)
{
	if(p_node == NULL)
		return NULL;

	return p_node->p_next;
}

void h_list_lookup_end(LINKED_LIST * p_linked_list)
{
	if(p_linked_list == NULL)
		return;

	giveup_ownership(p_linked_list);
}

LINKED_NODE * h_list_get_from_front(LINKED_LIST * p_list)
{
	return p_list->p_first_node;
}

LINKED_NODE * h_list_get_from_back(LINKED_LIST * p_list)
{
	return p_list->p_last_node;
}

BOOL h_list_is_empty(LINKED_LIST* p_list)
{
    return (p_list->p_first_node == NULL);
}



