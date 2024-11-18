/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irazafim <irazafim@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/07 12:13:11 by pmihangy          #+#    #+#             */
/*   Updated: 2024/11/18 17:39:53 by irazafim         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <minishell.h>

bool	print_error(char *str)
{
	if (str)
		write(2, str, ft_strlen(str));
	return (true);
}

bool	print_error_token(t_token *token, t_minishell *mshell)
{
	write(2, "syntax error near unexpected token ", 35);
	write(2, "'", 1);
	if (token->next == mshell->token)
		write(2, "newline", 7);
	else
		write(2, token->next->text, ft_strlen(token->next->text));
	write(2, "'\n", 2);
	return (false);
}

int	is_space(const char c)
{
	return (c == 32 || (c > 8 && c < 14));
}

void	handle_sigint(int signum)
{
	(void)signum;
	printf("\n");
	rl_replace_line("", 0);
	rl_on_new_line();
	rl_redisplay();
}

void	listen_signals(void)
{
	signal(SIGINT, handle_sigint);	
	signal(SIGQUIT, SIG_IGN);
}

void	init_minishell(t_minishell *mshell)
{
	mshell->token = NULL;
	mshell->env = NULL;
	mshell->cmd = NULL;
	mshell->exit_code = 0;
	mshell->pipefd[0] = -1;
	mshell->pipefd[1] = -1;
}

void	free_lst(t_lst **list)
{
	t_lst	*curr;
	t_lst	*tmp;
	
	if (*list == NULL)
		return ;
	curr = *list;
	while (curr->next != *list)
	{
		tmp = curr;
		curr = curr->next;
		free(tmp->text);
		free(tmp);
	}
	free(curr->text);
	free(curr);
	*list = NULL;
}

void	free_token(t_token **token)
{
	t_token	*curr;
	t_token	*tmp;
	
	if (*token == NULL)
		return ;
	curr = *token;
	while (curr->next != *token)
	{
		tmp = curr;
		curr = curr->next;
		free(tmp->text);
		free(tmp);
	}
	free(curr->text);
	free(curr);
	*token = NULL;
}

void	free_cmd(t_cmd **cmd)
{
	t_cmd	*curr;
	t_cmd	*tmp;
	
	if (*cmd == NULL)
		return ;
	curr = *cmd;
	while (curr->next != *cmd)
	{
		tmp = curr;
		curr = curr->next;
		free(tmp);
	}
	free(curr);
	*cmd = NULL;
}

t_lst	*new_lst_element(char *str)
{
	t_lst	*new;

	new = malloc(sizeof(t_lst));
	if (!new)
		return (NULL);
	new->text = ft_strdup(str);
	if (new->text == NULL)
		return (NULL);
	new->prev = NULL;
	new->next = NULL;
	return (new);
}

bool	lst_append(t_lst **env, char *str)
{
	t_lst	*new;
	
	new = new_lst_element(str);	
	if (new == NULL)
		return (false);
	if (*env == NULL)
	{
		*env = new;
		(*env)->prev = *env;
		(*env)->next = *env;
	}
	new->prev = (*env)->prev;
	new->next = (*env);
	(*env)->prev->next = new;
	(*env)->prev = new;
	return (true);
}

bool	init_env(t_minishell *mshell, char **env)
{
	int		i;
	char	*str;

	i = 0;
	while (env[i])
	{
		str = ft_strdup(env[i]);
		if (!str)
		{
			free_lst(&(mshell->env));
			return (false);
		}
		if (!lst_append(&(mshell->env), str))
			return (false);
		++i;
	}
	return (true);
}

void	print_lst(t_lst *env)
{
	t_lst	*curr;

	if (env == NULL)
		return ;
	curr = env;
	while (curr->next != env)
	{
		printf("%s\n", curr->text);		
		curr = curr->next;
	}
	printf("%s\n", curr->text);
}

void	print_token(t_token *token)
{
	t_token	*curr;

	if (token == NULL)
		return ;
	curr = token;
	while (curr->next != token)
	{
		printf("text: %s, id: %d\n", curr->text, curr->id);	
		curr = curr->next;
	}
	printf("text: %s, id: %d\n", curr->text, curr->id);
}

void	free_mshell(t_minishell *mshell)
{
	if (mshell->cmd)
		free_cmd(&(mshell->cmd));
	if (mshell->env)
		free_lst(&(mshell->env));
	if (mshell->token)
		free_token(&(mshell->token));
	if (mshell->pipefd[0] > 0)
		close(mshell->pipefd[0]);
	if (mshell->pipefd[1] > 0)
		close(mshell->pipefd[1]);
	rl_clear_history();
}

bool	is_empty(char *entry)
{
	int	i;
	bool flag;

	i = 0;
	flag = false;
	while(entry[i])
	{
		if(!is_space(entry[i]))
			flag = true;
		i++;
	}
	return (!flag);
}

size_t	len_list(t_lst *list)
{
	t_lst	*tmp;
	size_t	i;

	if ((list))
	{
		tmp = list;
		i = 1;
		while (tmp->next != list)
		{
			++i;
			tmp = tmp->next;
		}
		return (i);
	}
	return (0);
}
 
int	ft_search(char *str, char c)
{
	int	i;

	i = -1;
	while (str[++i])
		if (str[i] == c)
			return (i);
	return (0);
}

int	end_word(char *str, char *env)
{
	int	i;

	i = 0;
	while (str[i] && (ft_isalnum(str[i]) || str[i] == '_'))
		++i;
	return (ft_search(env, '='));
	// if (i == ft_search(env, '='))
	// 	return (i);
	// return (0);
}

/* return 1 si $VAR dans env sinon 0 */
int	exist_in_env(char *line, int *i, t_minishell *mshell)
{
	t_lst	*tmp;
	int		len;

	if (line[*i + 1] == '?' || line[*i + 1] == '$')
		return (2);
	tmp = mshell->env;
	len = len_list(tmp);
	while (len--)
	{
		if (ft_strncmp(tmp->text, &line[*i + 1], \
			end_word(&line[*i + 1], tmp->text)) == 0)
		{
			*i += ft_strlen(tmp->text) - \
				ft_strlen(ft_strchr(tmp->text, '=')) + 1;
			return (1);
		}
		tmp = tmp->next;
	}
	return (0);
}

char	*get_elem_env(t_lst *env, char *key)
{
	t_lst	*tmp;
	int		len;
	int		t;

	if (!key)
		return (NULL);
	tmp = env;
	len = len_list(tmp);
	t = ft_strlen(key);
	while (len--)
	{
		if (ft_strncmp(tmp->text, key, t) == 0)
		{
			len = 0;
			while (tmp->text[len])
				if (tmp->text[len++] == '=')
					break ;
			return (ft_strdup(&(tmp->text[len])));
		}
		tmp = tmp->next;
	}
	return (NULL);
}

char	*get_dollar_word(char *line, int size)
{
	char	*dollar;
	int		i;

	dollar = malloc(sizeof(char) * size);
	if (!dollar)
		return (NULL);
	i = 0;
	while (line[++i] && i < size)
		dollar[i - 1] = line[i];
	dollar[i - 1] = '\0';
	return (dollar);
}

int	in_env(t_minishell *mshell, char *line, int size, char **str)
{
	char	*tmp;
	char	*key;
	char	*value;

	key = get_dollar_word(line, size);
	value = get_elem_env(mshell->env, key);
	if (key)
		free(key);
	tmp = ft_strjoin(*str, value);
	if (value)
		free(value);
	free(*str);
	if (!tmp)
		return (0);
	*str = tmp;
	return (1);
}

int	dollar_point_interrogation(t_minishell *mshell, char **str)
{
	char	*tmp;
	char	*tmp2;

	tmp = ft_itoa(mshell->exit_code);
	if (!tmp)
		return (0);
	tmp2 = ft_strjoin(*str, tmp);
	free(tmp);
	free(*str);
	if (!tmp2)
		return (0);
	*str = tmp2;
	return (1);
}

int	add_dollar(char *line, int *index, char **str, t_minishell *mshell)
{
	int		ctrl;
	int		n;

	n = *index;
	ctrl = exist_in_env(line, index, mshell);
	if (ctrl == 1)
		return (in_env(mshell, &line[n], *index - n, str));
	else if (ctrl == 2)
	{
		(*index) += 2;
		return (dollar_point_interrogation(mshell, str));
	}
	else
	{
		++(*index);
		while (line[*index] && \
			(ft_isalnum(line[*index]) || line[*index] == '_'))
			++(*index);
		return (1);
	}
}

int	add_char(char *c, char **str, t_minishell *mshell, int *index)
{
	char	char_to_str[2];
	char	*tmp2;
	int		i;

	i = 0;
	if (c[i] == '$' && !mshell->sq && exist_in_env(c, &i, mshell))
		return (1);
	char_to_str[0] = *c;
	char_to_str[1] = '\0';
	(*index)++;
	tmp2 = ft_strjoin(*str, char_to_str);
	free(*str);
	if (!tmp2)
		return (0);
	*str = tmp2;
	return (1);
}

void	quoting_choice(bool *dq, bool *sq, int *index, char c)
{
	if ((c == '\'' || c == '"') && !*sq && !*dq)
	{
		if (c == '\'' && !*dq)
			*sq = true;
		else if (c == '"' && !*sq)
			*dq = true;
		if (index)
			++(*index);
	}
	else if ((c == '\'' || c == '"'))
	{
		if (c == '\'' && !*dq && *sq)
			*sq = false;
		else if (c == '"' && !*sq && *dq)
			*dq = false;
		if (index)
			++(*index);
	}
}

void	expand(char **entry, t_minishell *mshell)
{
	int		i;
	char	*entry_expanded;
	bool	dq;

	i = 0;
	dq = false;
	mshell->sq = false;
	entry_expanded = ft_strdup("");
	while ((*entry)[i])
	{
		quoting_choice(&dq, &mshell->sq, NULL, (*entry)[i]);
		if ((*entry)[i] && (*entry)[i + 1] && (*entry)[i] == '$' && \
			((*entry)[i + 1] != '\'' && (*entry)[i + 1] != '"') && \
			(ft_isalpha((*entry)[i + 1]) || (*entry)[i + 1] == '?' || \
			(*entry)[i + 1] == '_') && !mshell->sq)
			add_dollar(*entry, &i, &entry_expanded, mshell);
		if ((*entry)[i])
			add_char(&(*entry)[i], &entry_expanded, mshell, &i);
	}
	free(*entry);
	*entry = entry_expanded;
}

int	is_operator(char *str)
{
	if (str && *str && ft_strlen(str) >= 2)
	{
		if (!ft_strncmp(str, "<<", 2))
			return (HEREDOC);
		if (!ft_strncmp(str, ">>", 2))
			return (APPEND);
	}
	if (*str && ft_strlen(str) >= 1)
	{
		if (!ft_strncmp(str, "<", 1))
			return (INPUT);
		if (!ft_strncmp(str, ">", 1))
			return (TRUNC);
		if (!ft_strncmp(str, "|", 1))
			return (PIPE);
	}
	return (0);
}

int	length_cmd(char *command, int *quotes)
{
	int	i;

	i = 0;
	*quotes = 0;
	while (command[i] && !is_space(command[i]) && !is_operator(command + i))
	{
		if (command[i] == '"' || command[i] == '\'')
		{
			(*quotes)++;
			if (command[i++] == '"')
				while (command[i] && command[i] != '"')
					++i;
			else
				while (command[i] && command[i] != '\'')
					++i;
			if (command[i])
				++i;
		}
		if (command[i] && command[i] != '"' && command[i] != '\'' && \
			!is_space(command[i]) && !is_operator(command + i))
			++i;
	}
	return (i);
}

void	copy_token(char *command, int length, char *str, int i)
{
	int	j;

	j = 0;
	while (command[i + j] && i < length)
	{
		if (command[i + j] == '\'' && ++j)
		{
			while (command[i + j] != '\'' && ++i)
				str[i - 1] = command[(i - 1) + j];
			j++;
		}
		else if (command[i + j] == '"' && ++j)
		{
			while (command[i + j] != '"' && ++i)
				str[i - 1] = command[(i - 1) + j];
			j++;
		}
		else
		{
			str[i] = command[i + j];
			i++;
		}
	}
	str[i] = 0;
}

int	token_new_elem(t_token **new, char *str, int type)
{
	if (!str)
		return (0);
	(*new) = malloc(sizeof(t_token));
	if (*new == NULL)
	{
		free(str);
		return (0);
	}
	(*new)->text = str;
	(*new)->id = type;
	(*new)->next = NULL;
	(*new)->prev = NULL;
	return (1);
}

static void	add_first(t_token **list, t_token *new)
{
	(*list) = new;
	(*list)->prev = *list;
	(*list)->next = *list;
}

int	append_token(t_token **list, char *str, int type)
{
	t_token	*new;

	if (!token_new_elem(&new, str, type))
		return (0);
	if (!(*list))
		add_first(list, new);
	else
	{
		new->prev = (*list)->prev;
		new->next = (*list);
		(*list)->prev->next = new;
		(*list)->prev = new;
	}
	return (1);
}

bool	add_cmd(t_token **begin, char **command)
{
	char	*str;
	int		length;
	int		quotes;
	int		i;

	i = 0;
	length = length_cmd(*command, &quotes);
	if (((length) - (2 * quotes)) < 0)
		return (true);
	str = malloc(sizeof(char) * ((length + 1) - (2 * quotes)));
	if (!str)
		return (false);
	copy_token(*command, length - (2 * quotes), str, i);
	if (!append_token(begin, str, 0))
		return (false);
	if ((*begin)->prev == (*begin) || (*begin)->prev->prev->id == PIPE)
		(*begin)->prev->id = CMD;
	else
		(*begin)->prev->id = ARG;
	(*command) += length;
	return (true);
}

bool	add_special(t_token **begin, char **command)
{
	int	spe;

	spe = is_operator(*command);
	if (!spe)
		return (false);
	if (spe == INPUT && !append_token(begin, ft_strdup("<"), INPUT))
		return (false);
	else if (spe == HEREDOC && !append_token(begin, ft_strdup("<<"), HEREDOC))
		return (false);
	else if (spe == TRUNC && !append_token(begin, ft_strdup(">"), TRUNC))
		return (false);
	else if (spe == APPEND && !append_token(begin, ft_strdup(">>"), APPEND))
		return (false);
	else if (spe == PIPE && !append_token(begin, ft_strdup("|"), PIPE))
		return (false);
	if (spe == INPUT || spe == TRUNC || spe == PIPE)
		(*command)++;
	else if (spe == HEREDOC || spe == APPEND)
		(*command) += 2;
	return (true);
}

bool	tokenize(t_token **begin, char *command)
{
	(*begin) = NULL;
	while (*command)
	{
		while (is_space(*command))
			command++;
		if (*command && !is_operator(command) && !add_cmd(begin, &command))
		{
			if (*begin)
				free_token(begin);
			return (false);
		}
		else if (*command && is_operator(command) && \
					!add_special(begin, &command))
		{
			if (*begin)
				free_token(begin);
			return (false);
		}
	}
	return (true);
}


bool	read_in_stdin(t_minishell *mshell, int fd, char *word)
{
	char	*buf;

	while (1)
	{
		buf = NULL;
		buf = readline("> ");
		if (!buf)
		{
			print_error("warning: here-document delimited by end-of-file ");
			print_error("(wanted '");
			print_error(word);
			print_error("')\n");
			break ;
		}
		if (!ft_strncmp(word, buf, INT_MAX))
			break ;
		expand(&buf, mshell);
		// if (!expand(&buf, mshell))
		// 	free_mshell(mshell);
		write(fd, buf, ft_strlen(buf));
		write(fd, "\n", 1);
		free(buf);
	}
	free(buf);
	close(fd);
	return (true);
}


int	here_doc(t_minishell *mshell, char *word)
{
	int	fd;

	fd = open(".heredoc.tmp", O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd < 0)
		return (-1);
	if (!read_in_stdin(mshell, fd, word))
	{
		unlink(".heredoc.tmp");
		return (-1);
	}
	fd = open(".heredoc.tmp", O_RDONLY);
	if (fd > 0)
		unlink(".heredoc.tmp");
	return (fd);
}

int	open_file(t_minishell *mshell, char *filename, int type)
{
	int	fd;

	fd = -2;
	if (type == INPUT)
		fd = open(filename, O_RDONLY, 0644);
	else if (type == HEREDOC)
		fd = here_doc(mshell, filename);
	else if (type == TRUNC)
		fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	else if (type == APPEND)
		fd = open(filename, O_CREAT | O_WRONLY | O_APPEND, 0644);
	if (type != HEREDOC && fd < 0)
		perror(filename);
	return (fd);
}

bool	get_out(t_token *tmp, t_cmd *cmd, t_minishell *mshell)
{
	if (tmp->id == TRUNC)
	{
		if (cmd->out >= 0)
			close(cmd->out);
		if (tmp == tmp->next || tmp->next->id <= 5)
			return (print_error_token(tmp, mshell));
		cmd->out = open_file(NULL, tmp->next->text, TRUNC);
		if (cmd->out == -1)
			return (false);
	}
	else if (tmp->id == APPEND)
	{
		if (cmd->out >= 0)
			close(cmd->out);
		if (tmp == tmp->next || tmp->next->id <= 5)
			return (print_error_token(tmp, mshell));
		cmd->out = open_file(NULL, tmp->next->text, APPEND);
		if (cmd->out == -1)
			return (false);
	}
	return (true);
}

bool	get_outfile(t_token *token, t_cmd *cmd, t_minishell *mshell)
{
	t_token	*tmp;

	tmp = token;
	if (tmp->id != PIPE && !get_out(tmp, cmd, mshell))
		return (false);
	tmp = tmp->next;
	while (tmp != mshell->token && tmp->id != PIPE)
	{
		if (!get_out(tmp, cmd, mshell))
			return (false);
		tmp = tmp->next;
	}
	return (true);
}



bool	get_in(t_minishell *mshell, t_token *tmp, t_cmd *cmd)
{
	if (tmp->id == INPUT)
	{
		if (cmd->in >= 0)
			close(cmd->in);
		if (tmp == tmp->next || tmp->next->id <= 5)
			return (print_error_token(tmp, mshell));
		cmd->in = open_file(mshell, tmp->next->text, INPUT);
		if (cmd->in == -1)
			return (false);
	}
	else if (tmp->id == HEREDOC)
	{
		if (cmd->in >= 0)
			close(cmd->in);
		if (tmp == tmp->next || tmp->next->id <= 5)
			return (print_error_token(tmp, mshell));
		cmd->in = open_file(mshell, tmp->next->text, HEREDOC);
		if (cmd->in == -1)
			return (false);
	}
	return (true);
}

bool	get_infile(t_minishell *mshell, t_token *token, t_cmd *cmd)
{
	t_token	*tmp;

	tmp = token;
	if (tmp->id != PIPE && !get_in(mshell, tmp, cmd))
		return (false);
	if (tmp->id == PIPE)
		return (true);
	tmp = tmp->next;
	while (tmp->id != PIPE && tmp != mshell->token)
	{
		if (!get_in(mshell, tmp, cmd))
			return (false);
		tmp = tmp->next;
	}
	return (true);
}

static int	count_args(t_minishell *mshell, t_token *token)
{
	int		count;
	t_token	*tmp;

	count = 0;
	tmp = token;
	if (tmp->id == CMD || (tmp->id == ARG && \
		tmp->prev != mshell->token->prev && tmp->prev->id > 5))
		count ++;
	tmp = tmp->next;
	while (tmp != mshell->token && tmp->id != PIPE)
	{
		if (tmp->id == CMD || (tmp->id == ARG && \
		tmp->prev != mshell->token->prev && tmp->prev->id > 5))
			count ++;
		tmp = tmp->next;
	}
	return (count);
}

static int	add_to_cmd_param(char **cmd_param, int *i, char *str)
{
	cmd_param[*i] = ft_strdup(str);
	if (!cmd_param[*i])
		return (0);
	(*i)++;
	return (1);
}

static void	*free_cmd_param(char **cmd, int i)
{
	while (--i != -1)
		free(cmd[i]);
	free(cmd);
	return (NULL);
}

char	**get_param(t_minishell *mshell, t_token *token)
{
	char	**cmd_param;
	int		i;
	t_token	*tmp;

	i = 0;
	cmd_param = malloc(sizeof(char *) * (count_args(mshell, token) + 1));
	if (cmd_param == NULL)
		return (NULL);
	tmp = token;
	if (tmp->id != PIPE && (tmp->id == CMD || (tmp->id == ARG && \
		tmp->prev != mshell->token->prev && tmp->prev->id > 5)) && \
		!add_to_cmd_param(cmd_param, &i, tmp->text))
		return (free_cmd_param(cmd_param, i));
	tmp = tmp->next;
	while (tmp != mshell->token && tmp->id != PIPE)
	{
		if ((tmp->id == CMD || (tmp->id == ARG && \
			tmp->prev != mshell->token->prev && tmp->prev->id > 5)) && \
			!add_to_cmd_param(cmd_param, &i, tmp->text))
			return (free_cmd_param(cmd_param, i));
		tmp = tmp->next;
	}
	cmd_param[i] = NULL;
	return (cmd_param);
}


bool	fill_cmd(t_minishell *mshell, t_token *tmp)
{
	if (!get_infile(mshell, tmp, mshell->cmd->prev) && \
		mshell->cmd->prev->in != -1)
		return (false);
	if (mshell->cmd->prev->in == -1)
	{
		mshell->cmd->prev->skip_cmd = true;
		mshell->cmd->prev->out = -1;
		return (true);
	}
	if (!get_outfile(tmp, mshell->cmd->prev, mshell) && mshell->cmd->prev->out \
		!= -1)
		return (false);
	if (mshell->cmd->prev->out == -1)
	{
		if (mshell->cmd->prev->in >= 0)
			close (mshell->cmd->prev->in);
		mshell->cmd->prev->skip_cmd = true;
		mshell->cmd->prev->in = -1;
		return (true);
	}
	mshell->cmd->prev->cmd_param = get_param(mshell, tmp);
	if (!mshell->cmd->prev->cmd_param)
		free_mshell(mshell);
	return (true);
}

int	cmd_new_elem(t_cmd **new, int infile, int outfile, char **cmd_param)
{
	(*new) = malloc(sizeof(t_cmd));
	if (*new == NULL)
		return (0);
	(*new)->skip_cmd = false;
	(*new)->in = infile;
	(*new)->out = outfile;
	(*new)->cmd_param = cmd_param;
	(*new)->next = NULL;
	(*new)->prev = NULL;
	return (1);
}

int	append_cmd(t_cmd **list, int infile, int outfile, char **cmd_param)
{
	t_cmd	*new;

	if (!cmd_new_elem(&new, infile, outfile, cmd_param))
		return (0);
	if (!(*list))
	{
		(*list) = new;
		(*list)->prev = *list;
		(*list)->next = *list;
	}
	else
	{
		new->prev = (*list)->prev;
		new->next = (*list);
		(*list)->prev->next = new;
		(*list)->prev = new;
	}
	return (1);
}

bool	norm(t_minishell *mshell, t_token *tmp)
{
	if (!append_cmd(&mshell->cmd, -2, -2, NULL))
		free_mshell(mshell);
	if (!fill_cmd(mshell, tmp))
	{
		mshell->exit_code = 2;
		return (false);
	}
	return (true);
}

bool	create_list_cmd(t_minishell *mshell)
{
	t_token	*tmp;

	tmp = mshell->token;
	if (!norm(mshell, tmp))
		return (false);
	tmp = tmp->next;
	while (tmp != mshell->token)
	{
		if (tmp->prev->id == PIPE)
		{
			if (!norm(mshell, tmp))
				return (false);
		}
		tmp = tmp->next;
	}
	return (true);
}

void	parse(t_minishell *mshell, char *entry)
{
	char	*err_message;
	// TODO:  manage if has an error
	expand(&entry, mshell);
	tokenize(&mshell->token, entry);
	if (mshell->token && mshell->token->prev->id == PIPE)
	{
		err_message = ft_strdup("Syntax error\n");
		if (!err_message)
			return ;
		write(2, err_message, ft_strlen(err_message));
		mshell->exit_code = 2;
		free_token(&mshell->token);
		return ;
	}
	if (!mshell->token)
		create_list_cmd(mshell);
	// print_list_cmd(mshell->cmd);
}

int	main(int ac, char **av, char **env)
{
	char		*entry;
	t_minishell	mshell;
	
	(void)ac;
	(void)av;
	entry = NULL;
	init_minishell(&mshell);
	if (!init_env(&mshell, env))
	{
		free_mshell(&mshell);
		return (1);
	}
	listen_signals();
	while (1)
	{
		entry = readline("minishell> ");
		if (entry == NULL)
		{
			printf("exit\n");
			exit(1);
		}
		if (has_open_quote(entry, false, 0))
			printf("open quote\n");
		else if (is_empty(entry))
			continue ;
		else
		{
			add_history(entry);
			parse(&mshell, entry);
			// exec(&mshell);
		}
		free_token(&mshell.token);
		free_cmd(&mshell.cmd);
	}
	rl_clear_history();
	free_mshell(&mshell);
	return (0);
}