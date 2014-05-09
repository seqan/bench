library("reshape")
library("xtable")
library("ggplot2")
library("scales")

PATH="/Users/esiragusa/Datasets/ibench"

FILENAME_VISIT=paste(paste(PATH, "visit", sep='/'), "tsv", sep='.')
FILENAME_QUERY=paste(paste(PATH, "query", sep='/'), "tsv", sep='.')
PLOT_VISIT=paste(paste(PATH, "visit", sep='/'), "pdf", sep='.')
PLOT_QUERY=paste(paste(PATH, "query", sep='/'), "pdf", sep='.')

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

table_visit = subset(TABLE_VISIT, alphabet=='dna', select=c(index, depth, time))
table_nodes = subset(TABLE_VISIT, alphabet=='dna' & index=='fm-tl', select=c(depth, nodes))
table_nodes[,'nodes'] = log(table_nodes[, 'nodes'], 4)

ggplot() +
  geom_line(data=table_visit, aes(x=depth, y=log10(time), group=index, shape=index, color=index)) +
  geom_point(data=table_visit, aes(x=depth, y=log10(time), group=index, shape=index, color=index), size=3) +
  geom_line(data=table_nodes, aes(x=depth, y=nodes/4), linetype='dotted') +
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

table_query_0 = subset(TABLE_QUERY, alphabet=='dna' & errors==0, select=c(index, plength, time))
table_query_1 = subset(TABLE_QUERY, alphabet=='dna' & errors==1, select=c(index, plength, time))

ggplot() +
  geom_line(data=table_query_0, aes(x=plength, y=time, group=index, shape=index, color=index)) +
  geom_point(data=table_query_0, aes(x=plength, y=time, group=index, shape=index, color=index), size=3) +
  geom_line(data=table_query_1, aes(x=plength, y=time, group=index, shape=index, color=index), linetype='dashed') +  
  geom_point(data=table_query_1, aes(x=plength, y=time, group=index, shape=index, color=index), size=3) +
  geom_line(data=table_nodes, aes(x=depth, y=nodes), linetype='dotted') +
  xlab("pattern length") +
  ylab("microseconds") +
  theme_bw(base_size=12, base_family="Helvetica")

ggsave(file=PLOT_QUERY)



# table_occ = subset(TABLE_QUERY, alphabet=='dna' & index=='fm-tl', select=c(plength, errors, occurrences))
# table_occ[,'occurrences'] = table_occ[, 'occurrences'] / 1000000
# 
# ggplot() +
#   geom_line(data=table_occ, aes(x=plength, y=occurrences, group=errors, color=errors)) +
#   xlab("pattern length") +
#   ylab("occurrences") +
#   scale_y_log10() +
#   ggtitle("Occurrenecs by pattern length") +
#   theme_bw(base_size=12, base_family="Helvetica")

