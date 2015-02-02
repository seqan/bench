library("reshape")
library("ggplot2")
library("ggthemes")
library("scales")
library("sitools")
library("Cairo")

INPUT="/Volumes/barracuda/Code/seqan/apps/bench/scripts/resources"
OUTPUT="/Users/esiragusa/Documents/Dissertation/plots"

SCALE=0.4
FONT_SIZE=10
POINT_SIZE=2
FONT_FAMILY='Cambria'

INDEX_LABELS = c(
    'esa'="ESA",
    'lst'="LST",
    'sa'="SA",
    'qgram'="q-Gram",
    'fm-wt'="FM-WT",
    'fm-tl'="FM-TL"
)
INDEX_COLORS <- c("#2CA02C", "#8C564B", "#D62728", "#FF7F0E","#9467BD", "#1F77B4")
names(INDEX_COLORS) <- names(INDEX_LABELS)
INDEX_SHAPES <- c(0,1,2,3,4,5)
names(INDEX_SHAPES) <- names(INDEX_LABELS)

INDEX_SCALING=c('sa', 'fm-tl')

ALGORITHM_NAMES=c("single","sort","dfs")   #,"bfs")
ALGORITHM_LABELS=c("Single","Sorted","Multiple") #,"Bfs")
names(ALGORITHM_LABELS) <- ALGORITHM_NAMES
ALGORITHM_COLORS <- c("#1F77B4", "#2CA02C", "#D62728")
names(ALGORITHM_COLORS) <- ALGORITHM_NAMES

DATASET='celegans'
ALPHABET='dna'
ALPHSIZE=4
PLENGTHS=c(15,30)

#DATASET='uniprot'
#ALPHABET='protein'
#ALPHSIZE=24
#PLENGTHS=c(10,20)


QUERY_BREAKS = list(
  c(0,1,2,3) * 10^-6,
  c(0,2,4,6) * 10^-5
)

MULTI_BREAKS = list(
  c(0,0.5,1,1.5) * 10^-6,
  c(0,2,4,6) * 10^-5
)

### FUNCTIONS ###

options(scipen=999)

f2si_u <- function(x)
{
  f2si(x, unit='u')
}

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

FILENAME_VISIT=paste(paste(INPUT, "visit", sep='/'), "tsv", sep='.')
PLOT_VISIT=paste(paste(OUTPUT, "visit", sep='/'), ALPHABET, DATASET, "pdf", sep='.')

if ((R = load_file(FILENAME_VISIT))$ok)
{
  TABLE_VISIT = R$tsv;
} else
  print(paste("NOT FOUND:", FILENAME_VISIT))

table_visit = subset(TABLE_VISIT, alphabet==ALPHABET & dataset==DATASET, select=c(index, depth, time))
#table_nodes = subset(TABLE_VISIT, alphabet==ALPHABET & dataset==DATASET & index=='fm-wt', select=c(depth, nodes))
#table_nodes[,'nodes'] = 10^log(table_nodes[, 'nodes'], ALPHSIZE)
#  geom_line(data=table_nodes, aes(x=depth, y=nodes/ALPHSIZE), linetype='dotted') +

ggplot() +
  geom_line(data=table_visit, aes(x=depth, y=time, group=index, shape=index, color=index)) +
  geom_point(data=table_visit, aes(x=depth, y=time, group=index, shape=index, color=index), size=POINT_SIZE) +
  scale_shape_manual(name="Index", breaks=names(INDEX_LABELS), labels=INDEX_LABELS, values=INDEX_SHAPES) +
  scale_color_manual(name="Index", breaks=names(INDEX_LABELS), labels=INDEX_LABELS, values=INDEX_COLORS) +
  scale_y_log10(labels=f2si) +
  xlab("Depth") +
  ylab("Time [s]") +
  theme_bw(base_size=FONT_SIZE, base_family=FONT_FAMILY)

ggsave(file=PLOT_VISIT, scale=SCALE, device=cairo_pdf)


### PLOT QUERY ###

FILENAME_QUERY=paste(paste(INPUT, "query", sep='/'), "tsv", sep='.')

if ((R = load_file(FILENAME_QUERY))$ok)
{
  TABLE_QUERY = R$tsv;
} else
  print(paste("NOT FOUND:", FILENAME_QUERY))

for (ERRORS in 0:1)
{
  PLOT_QUERY=paste(paste(OUTPUT, "query", sep='/'), ALPHABET, DATASET, ERRORS, "pdf", sep='.')

  #table_query = subset(TABLE_QUERY, alphabet==ALPHABET & dataset==DATASET & errors==ERRORS, select=c(index, plength, pcount, time))
  #table_query <- transform(table_query, time = time / pcount)
  table_query = subset(TABLE_QUERY, alphabet==ALPHABET & dataset==DATASET & errors==ERRORS, select=c(index, plength, time))
  table_query <- transform(table_query, time = time / 1000000)
  
  ggplot() +
    geom_line(data=table_query, aes(x=plength, y=time, group=index, shape=index, color=index)) +
    geom_point(data=table_query, aes(x=plength, y=time, group=index, shape=index, color=index), size=POINT_SIZE) +
    scale_shape_manual(name="Index", breaks=names(INDEX_LABELS), labels=INDEX_LABELS, values=INDEX_SHAPES) +
    scale_color_manual(name="Index", breaks=names(INDEX_LABELS), labels=INDEX_LABELS, values=INDEX_COLORS) +
    xlab("Pattern length") +
    ylab("Time [s]") +
    scale_y_continuous(breaks=QUERY_BREAKS[[ERRORS+1]], labels=f2si) +
    theme_bw(base_size=FONT_SIZE, base_family=FONT_FAMILY)

  ggsave(file=PLOT_QUERY, scale=SCALE, device=cairo_pdf)
}


### PLOT MULTI QUERY ###

FILENAME_MULTI=paste(paste(INPUT, "multi", sep='/'), "tsv", sep='.')

if ((R = load_file(FILENAME_MULTI))$ok)
{
  TABLE_MULTI = R$tsv;
} else
  print(paste("NOT FOUND:", FILENAME_MULTI))

TABLE_MULTI = subset(TABLE_MULTI, algorithm != 'bfs')

for (ERRORS in 0:1)
{
    PLENGTH=PLENGTHS[ERRORS+1]
    PLOT_MULTI=paste(paste(OUTPUT, "multi", sep='/'), ALPHABET, DATASET, ERRORS, PLENGTH, "pdf", sep='.')
    
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
      scale_fill_manual(name="Index", breaks=names(INDEX_LABELS), labels=INDEX_LABELS, values=INDEX_COLORS) +
      scale_color_manual(name="Index", breaks=names(INDEX_LABELS), labels=INDEX_LABELS, values=INDEX_COLORS) +
      scale_x_discrete(breaks=ALGORITHM_NAMES, labels=ALGORITHM_LABELS) +
      scale_y_continuous(breaks=MULTI_BREAKS[[ERRORS+1]], labels=f2si) +
      xlab("Algorithm") +
      ylab("Average time per pattern [s]") +
      theme_bw(base_size=FONT_SIZE, base_family=FONT_FAMILY)
      
    ggsave(file=PLOT_MULTI, scale=SCALE, device=cairo_pdf)
}


ERRORS=1
PLENGTH=PLENGTHS[ERRORS+1]
for (INDEX in INDEX_SCALING)
{
  PLOT_MULTI_IDX=paste(paste(OUTPUT, "multi", sep='/'), ALPHABET, DATASET, ERRORS, INDEX, "pdf", sep='.')
  table_multi_idx = subset(TABLE_MULTI, alphabet==ALPHABET & dataset==DATASET & errors==ERRORS & plength==PLENGTH & index==INDEX, select=c(algorithm, pcount, time, preprocessing))
  
  table_multi_idx_t <- transform(table_multi_idx, time = time / pcount)
  table_multi_idx_p <- transform(table_multi_idx, time = (time + preprocessing) / pcount)
  
  table_multi_idx_breaks=c(10000,100000,1000000,10000000)
  table_multi_idx_labels=c('10k', '100k', '1M', '10M')

  ggplot() +
    geom_line(data=table_multi_idx_p, aes(x=pcount, y=time, group=algorithm, shape=algorithm, color=algorithm), linetype='solid') +
    geom_point(data=table_multi_idx_p, aes(x=pcount, y=time, group=algorithm, shape=algorithm, color=algorithm), size=POINT_SIZE) +
    geom_line(data=table_multi_idx_t, aes(x=pcount, y=time, group=algorithm, shape=algorithm, color=algorithm), linetype='dashed') +
    geom_point(data=table_multi_idx_t, aes(x=pcount, y=time, group=algorithm, shape=algorithm, color=algorithm), size=POINT_SIZE-1) +
    scale_shape_discrete(name="Algorithm", breaks=ALGORITHM_NAMES, labels=ALGORITHM_LABELS) +
    scale_color_manual(name="Algorithm", breaks=ALGORITHM_NAMES, labels=ALGORITHM_LABELS, values=ALGORITHM_COLORS) +
    xlab("Patterns") +
    ylab("Average time per pattern [s]") +
    scale_x_log10(breaks=table_multi_idx_breaks, labels=table_multi_idx_labels) +
    scale_y_continuous(labels=f2si) +
    theme_bw(base_size=FONT_SIZE, base_family=FONT_FAMILY)

  ggsave(file=PLOT_MULTI_IDX, scale=SCALE, device=cairo_pdf)
}


