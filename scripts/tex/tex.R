library("reshape")
library("xtable")
library("ggplot2")
library("scales")

PATH="/Users/esiragusa/Code/seqan/core/apps/ibench/scripts/resources"
ALPHABET='dna'
ALPHSIZE=4
DATASET='ecoli'


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
PLOT_VISIT=paste(paste(PATH, "visit", sep='/'), ALPHABET, DATASET, "pdf", sep='.')

if ((R = load_file(FILENAME_VISIT))$ok)
{
  TABLE_VISIT = R$tsv;
} else
  print(paste("NOT FOUND:", FILENAME_VISIT))

table_visit = subset(TABLE_VISIT, alphabet==ALPHABET & dataset==DATASET, select=c(index, depth, time))
table_nodes = subset(TABLE_VISIT, alphabet==ALPHABET & dataset==DATASET & index=='fm-wt', select=c(depth, nodes))
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
  PLOT_QUERY=paste(paste(PATH, "query", sep='/'), ALPHABET, DATASET, ERRORS, "pdf", sep='.')

  table_query = subset(TABLE_QUERY, alphabet==ALPHABET & dataset==DATASET & errors==ERRORS, select=c(index, plength, pcount, time))
  table_query <- transform(table_query, time = time / pcount)
  
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

PLENGTHS=c(15,30)
for (ERRORS in 0:1)
{
    PLENGTH=PLENGTHS[ERRORS+1]
    PLOT_MULTI=paste(paste(PATH, "multi", sep='/'), ALPHABET, DATASET, ERRORS, PLENGTH, "pdf", sep='.')
    
    table_multi = subset(TABLE_MULTI, alphabet==ALPHABET & dataset==DATASET & errors==ERRORS, select=c(index, algorithm, plength, pcount, time, preprocessing))
    table_multi$algorithm <- factor(table_multi$algorithm, levels=table_multi$algorithm, ordered = TRUE)
    
    table_multi_t <- transform(table_multi, time = time / pcount)
    table_multi_p <- transform(table_multi, time = (time + preprocessing) / pcount)
    
    PCOUNT=max(table_multi_t['pcount'])
    
    table_multi_t = subset(table_multi_t, pcount==PCOUNT & plength==PLENGTH, select=c(index, algorithm, time))
    table_multi_p = subset(table_multi_p, pcount==PCOUNT & plength==PLENGTH, select=c(index, algorithm, time))
    
    ggplot() +
      geom_bar(data=table_multi_p, aes(algorithm, y=time, colour=index), position="dodge", stat="identity", ordered=TRUE) +
      geom_bar(data=table_multi_t, aes(algorithm, y=time, fill=index), position="dodge", stat="identity", ordered=TRUE) +
      ylab("seconds") +
      theme_bw(base_size=12, base_family="Helvetica")
      
    ggsave(file=PLOT_MULTI)
}


ERRORS=1
PLENGTH=30
for (INDEX in c('sa', 'fm-tl'))
{
  PLOT_MULTI_IDX=paste(paste(PATH, "multi", sep='/'), ALPHABET, DATASET, ERRORS, INDEX, "pdf", sep='.')
  table_multi_idx = subset(TABLE_MULTI, alphabet==ALPHABET & dataset==DATASET & errors==ERRORS & plength==PLENGTH & index==INDEX, select=c(algorithm, pcount, time, preprocessing))
  
  table_multi_idx_t <- transform(table_multi_idx, time = time / pcount)
  table_multi_idx_p <- transform(table_multi_idx, time = (time + preprocessing) / pcount)
  
  ggplot() +
    geom_line(data=table_multi_idx_p, aes(x=pcount, y=time, group=algorithm, shape=algorithm, color=algorithm), linetype='solid') +
    geom_point(data=table_multi_idx_p, aes(x=pcount, y=time, group=algorithm, shape=algorithm, color=algorithm), size=3) +
    geom_line(data=table_multi_idx_t, aes(x=pcount, y=time, group=algorithm, shape=algorithm, color=algorithm), linetype='dashed') +
    geom_point(data=table_multi_idx_t, aes(x=pcount, y=time, group=algorithm, shape=algorithm, color=algorithm), size=2) +
    xlab("pattern count") +
    ylab("seconds") +
    scale_x_log10() +
    theme_bw(base_size=12, base_family="Helvetica")

  ggsave(file=PLOT_MULTI_IDX)
}


