library("reshape")
library("xtable")
library("ggplot2")
library("scales")

PATH="/Users/esiragusa/Code/seqan/core/apps/ibench/scripts/resources"
ALPHABET='dna'
ALPHSIZE=4


### FUNCTIONS ###

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

FILENAME_VISIT=paste(paste(PATH, "visit", sep='/'), "tsv", sep='.')
PLOT_VISIT=paste(paste(PATH, "visit", sep='/'), ALPHABET, "pdf", sep='.')

if ((R = load_file(FILENAME_VISIT))$ok)
{
  TABLE_VISIT = R$tsv;
} else
  print(paste("NOT FOUND:", FILENAME_VISIT))

table_visit = subset(TABLE_VISIT, alphabet==ALPHABET, select=c(index, depth, time))
table_nodes = subset(TABLE_VISIT, alphabet==ALPHABET & index=='fm-wt', select=c(depth, nodes))
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

FILENAME_QUERY=paste(paste(PATH, "query", sep='/'), "tsv", sep='.')

if ((R = load_file(FILENAME_QUERY))$ok)
{
  TABLE_QUERY = R$tsv;
} else
  print(paste("NOT FOUND:", FILENAME_QUERY))

for (ERRORS in 0:1)
{
  PLOT_QUERY=paste(paste(PATH, "query", sep='/'), ALPHABET, ERRORS, "pdf", sep='.')

  table_query = subset(TABLE_QUERY, alphabet==ALPHABET & errors==ERRORS, select=c(index, plength, time))
#  pcount = subset(TABLE_QUERY, alphabet==ALPHABET & errors==ERRORS & index=='fm-wt', select=c(pcount))
  pcount = 1000000
  table_query[,'time'] = table_query[,'time'] / pcount
  
  ggplot() +
    geom_line(data=table_query, aes(x=plength, y=time, group=index, shape=index, color=index)) +
    geom_point(data=table_query, aes(x=plength, y=time, group=index, shape=index, color=index), size=3) +
    xlab("pattern length") +
    ylab("seconds") +
    theme_bw(base_size=12, base_family="Helvetica")

  ggsave(file=PLOT_QUERY)
}


### PLOT MULTI QUERY ###

FILENAME_MULTI=paste(paste(PATH, "multi", sep='/'), "tsv", sep='.')

if ((R = load_file(FILENAME_MULTI))$ok)
{
  TABLE_MULTI = R$tsv;
} else
  print(paste("NOT FOUND:", FILENAME_MULTI))

PCOUNT=1000000
for (ERRORS in 0:1)
{
  for (PLENGTH in c(15,30))
  {
    PLOT_MULTI=paste(paste(PATH, "multi", sep='/'), ALPHABET, ERRORS, PLENGTH, "pdf", sep='.')
    
    table_multi = subset(TABLE_MULTI, alphabet==ALPHABET & errors==ERRORS, select=c(index, algorithm, plength, pcount, time, preprocessing))
    table_multi <- transform(table_multi, time = (time + preprocessing) / pcount)

    table_multi_bar = subset(table_multi, pcount==PCOUNT & plength==PLENGTH, select=c(index, algorithm, time))
    table_multi_bar$algorithm <- factor(table_multi_bar$algorithm, levels=table_multi_bar$algorithm, ordered = TRUE)

    ggplot() +
      geom_bar(data=table_multi_bar, aes(algorithm, y=time, fill=index), position="dodge", stat="identity", ordered=TRUE) +
      theme_bw(base_size=12, base_family="Helvetica")
    
    ggsave(file=PLOT_MULTI)
  }
}

#  table_multi = subset(table_multi, index=='fm-tl' | index=='sa' | index=='qgram')
#  table_multi_single = subset(table_multi, algorithm=='single' & plength==PLENGTH, select=c(index, pcount, time))
#  table_multi_sort = subset(table_multi, algorithm=='sort' & plength==PLENGTH, select=c(index, pcount, time))
#  table_multi_dfs = subset(table_multi, algorithm=='dfs' & plength==PLENGTH, select=c(index, pcount, time))
#ggplot() +
#  geom_line(data=table_multi_single, aes(x=pcount, y=time, group=index, shape=index, color=index), linetype='solid') +
#  geom_line(data=table_multi_sort, aes(x=pcount, y=time, group=index, shape=index, color=index), linetype='longdash') +  
#  geom_line(data=table_multi_dfs, aes(x=pcount, y=time, group=index, shape=index, color=index), linetype='dotdash') +
#  xlab("patterns count") +
#  ylab("seconds") +
#  scale_x_continuous(trans=log_trans(), breaks=c(10000,100000,1000000)) +
#  theme_bw(base_size=12, base_family="Helvetica")

#}


