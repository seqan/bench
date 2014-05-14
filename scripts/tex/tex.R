library("reshape")
library("xtable")
library("ggplot2")
library("scales")

PATH="/Users/esiragusa/Code/seqan/core/apps/ibench/scripts/resources"
ALPHABET='dna'
ALPHSIZE=4

FILENAME_VISIT=paste(paste(PATH, "visit", sep='/'), "tsv", sep='.')
FILENAME_QUERY=paste(paste(PATH, "query", sep='/'), "tsv", sep='.')
PLOT_VISIT=paste(paste(PATH, "visit", sep='/'), ALPHABET, "pdf", sep='.')
PLOT_QUERY_0=paste(paste(PATH, "query", sep='/'), ALPHABET, "0", "pdf", sep='.')
PLOT_QUERY_1=paste(paste(PATH, "query", sep='/'), ALPHABET, "1", "pdf", sep='.')
PLOT_QUERY_2=paste(paste(PATH, "query", sep='/'), ALPHABET, "2", "pdf", sep='.')

options(scipen=999)

checkNA <- function(y)
{
    any(is.na(y))
}

load_file <- function(filename)
{
    if (file.exists(filename))
    {
        # try the main resource file first
        print(paste("read",filename))
        tsvFile = try(read.delim(filename, header=TRUE))
        if (inherits(tsvFile,"try-error"))
        {
            print(paste("EMPTY FILE:", filename))
            return(list(ok=FALSE))
        }
        if (nrow(tsvFile) < 1)
        {
            print(paste("BAD FORMAT:", filename))
            return(list(ok=FALSE))
        }
        if (any(apply(tsvFile, 1, checkNA)))
        {
            print(paste("BAD FORMAT:", filename))
            return(list(ok=FALSE))
        }
        return(list(ok=TRUE,tsv=tsvFile))
    } else
        return(list(ok=FALSE))
}


### PLOT VISIT ###

if ((R = load_file(FILENAME_VISIT))$ok)
{
  TABLE_VISIT = R$tsv;
} else
  print(paste("NOT FOUND:", FILENAME_VISIT))

table_visit = subset(TABLE_VISIT, alphabet==ALPHABET, select=c(index, depth, time))
table_nodes = subset(TABLE_VISIT, alphabet==ALPHABET & index=='fm-tl', select=c(depth, nodes))
table_nodes[,'nodes'] = log(table_nodes[, 'nodes'], ALPHSIZE)

ggplot() +
  geom_line(data=table_visit, aes(x=depth, y=log10(time), group=index, shape=index, color=index)) +
  geom_point(data=table_visit, aes(x=depth, y=log10(time), group=index, shape=index, color=index), size=3) +
  geom_line(data=table_nodes, aes(x=depth, y=nodes/ALPHSIZE), linetype='dotted') +
  xlab("depth") +
  ylab("log10 seconds") +
  theme_bw(base_size=12, base_family="Helvetica")
  
ggsave(file=PLOT_VISIT)


### PLOT QUERY ###

if ((R = load_file(FILENAME_QUERY))$ok)
{
  TABLE_QUERY = R$tsv;
} else
  print(paste("NOT FOUND:", FILENAME_QUERY))

table_query_0 = subset(TABLE_QUERY, alphabet==ALPHABET & errors==0, select=c(index, plength, time))
table_query_1 = subset(TABLE_QUERY, alphabet==ALPHABET & errors==1, select=c(index, plength, time))
table_query_2 = subset(TABLE_QUERY, alphabet==ALPHABET & errors==2, select=c(index, plength, time))

ggplot() +
  geom_line(data=table_query_0, aes(x=plength, y=time, group=index, shape=index, color=index)) +
  geom_point(data=table_query_0, aes(x=plength, y=time, group=index, shape=index, color=index), size=3) +
  xlab("pattern length") +
  ylab("microseconds") +
  theme_bw(base_size=12, base_family="Helvetica")

ggsave(file=PLOT_QUERY_0)

ggplot() +
  geom_line(data=table_query_1, aes(x=plength, y=time, group=index, shape=index, color=index)) +  
  geom_point(data=table_query_1, aes(x=plength, y=time, group=index, shape=index, color=index), size=3) +
  xlab("pattern length") +
  ylab("microseconds") +
  theme_bw(base_size=12, base_family="Helvetica")

ggsave(file=PLOT_QUERY_1)

ggplot() +
  geom_line(data=table_query_2, aes(x=plength, y=time, group=index, shape=index, color=index)) +  
  geom_point(data=table_query_2, aes(x=plength, y=time, group=index, shape=index, color=index), size=3) +
  xlab("pattern length") +
  ylab("microseconds") +
  theme_bw(base_size=12, base_family="Helvetica")

ggsave(file=PLOT_QUERY_2)
