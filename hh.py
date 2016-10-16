bool deFirstLine(ByteBlob firstLine);
    HttpVersion getVersion(){return m_version;};
        string getHeader(string key);
            string getAllHeaders();
                void setVersion(HttpVersion httpVersion){m_version = httpVersion;};
                    void setHeader(string key, string value);
                        void decodeHeaderLine(ByteBlob headerLines);

HWK 3:  Assigned 10/13   Due Date window ends 10-20 (9:55 am -- Before the 10 am class)
    
    
    1. Exercise 4 on page 107
        2. Exercise 11 on page 110
            3. Exercise 7 on page 191
                4. Exercise 11 on Page 193
                    5. A Hamiltonian path in a graph is a path that has no repeated vertices and covers all vertices.
Design an efficient algorithm  for finding a Hamiltonian path in a DAG.
    6.  Let G = (V,E) be a connected undirected graph and let v be a vertex in G. Let T by the depth-first
search tree of G starting from v, and let U be the breadth-first search tree of G starting from v.
Prove that the depth of T is at least as great as the depth of U.
}